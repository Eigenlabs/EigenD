
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

/* --------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * piw_consolemixer.cpp
 *
 * Implementation of a console style mixer with input channels and effect send/return channels.
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include <piw/piw_mixer.h>
#include <piw/piw_clock.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>
#include <picross/pic_float.h>
#include <map>
#include <string>
#include <sstream>

#define CLKSTATE_IDLE 0
#define CLKSTATE_SHUTDOWN 1
#define CLKSTATE_RUNNING 2
#define CLKSTATE_STARTUP 3

// TODO: tasks                      status
//  1. volume, pan on inputs        done
//  2. send levels                  done
//  3. handle clock loop            done
//  4. test removing chans          done
//  5. feedback saturation
//  6. constantly settings gains

// debug level
// 0: off
// 1: lo
// 2: hi
#define CONSOLE_MIXER_DEBUG 0


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace
{
    struct fx_node_t;
    struct consolemixer_chan_t;
    struct consolemixer_fx_chan_t;
    struct consolemixer_input_clock_t;
    struct consolemixer_output_clock_t;
    struct consolemixer_chan_audiowire_t;
    struct consolemixer_controlwire_t;
    struct consolemixer_sendwire_t;
    struct consolemixer_master_controls_t;
} // namespace

/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * struct consolemixer_t::impl_t: implementation of the main console mixer
 *
 * Behaves like an aggregator; creating a channel returns a cookie to a new input channel root_t.
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

struct piw::consolemixer_t::impl_t: piw::root_ctl_t, piw::wire_ctl_t, piw::clocksink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
{
    impl_t(const pic::f2f_t &vol, const pic::f2f_t &pan, piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
    ~impl_t() { tracked_invalidate(); invalidate(); }
    void invalidate();

    // event_data_source_real_t inherited functions:
    void source_ended(unsigned seq) { }

    // get browseable state

    // clocksink_t inherited functions:
    void clocksink_ticked(unsigned long long from, unsigned long long t);
    // perform the mixing
    piw::data_nb_t mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float gain, unsigned long long t, unsigned bs);
    // add channel buffers to the channel mix queue
    void add_to_mix_queues(unsigned audio_channel, const piw::data_nb_t &buffer);
    // add channel buffers to an fx channel send queue
    void add_to_fx_send_queues(unsigned fx_chan_num, unsigned audio_channel, const piw::data_nb_t &buffer);

    // create a mixer input channel
    piw::cookie_t create_channel(unsigned num);
    // remove a mixer input channel
    void remove_channel(unsigned num);
    // create a mixer fx channel
    piw::cookie_t create_fx_channel(unsigned num, const piw::cookie_t &output_cookie);
    // remove a mixer fx channel
    void remove_fx_channel(unsigned num);
    // enable or disable a send between a channel (input or effect) and an effect channel
    void set_fx_send_enable(bool enable, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan);
    // 
    void set_fx_send_prefader(bool enable, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan);
    // update latency by getting latency of all channels
    void update_latency();
    // provide cookie for the master controls (vol, pan, etc.)
    piw::cookie_t master_controls_cookie();

    void set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan);

    // clock state
    int clk_state_;
    // clock domain
    piw::clockdomain_ctl_t *clk_domain_;

    // number of audio channels
    unsigned num_audio_chans_;

    // queue of channels to mix
    pic::lckvector_t<pic::lcklist_t<piw::data_nb_t>::nbtype>::nbtype chan_mix_queue_;

    // output audio buffer
    piw::xevent_data_buffer_t output_buffer_;

    // console channels
    std::map<unsigned, consolemixer_chan_t *> channels_;

    // console fx channels
    std::map<unsigned, consolemixer_fx_chan_t *> fx_channels_;

    // master controls root
    consolemixer_master_controls_t *master_controls_;

    // left and right master gains
    float lgain_, rgain_;

    // volume and pan control level to gain functions
    pic::f2f_t volfunc_;
    pic::f2f_t panfunc_;
};


namespace
{
    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct fx_node_t: node for constructing a fx channel send graph
     *
     * The fx channel clock graph must be acyclic, but cycles are introduced when one channel
     * sends to another, and the other sends back again either directly or through other sends paths.
     * These nodes are used to construct a graph to detect these cycles before attempting to make
     * a clock sink relationship. This prevents external effect connections from being blocked due to
     * a clock cycle in the effect sends.
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct fx_node_t
    {
        fx_node_t(unsigned chan_num) : walk_flag_(false), chan_num_(chan_num) {}
        ~fx_node_t() {}

        bool add_upstream_fx_node(fx_node_t *node)
        {
            // check if edge already exists
            bool success = true;
            for(unsigned i=0;i<upstream_edges_.size();i++)
            {
                if(upstream_edges_[i]==node)
                {
                    // already an edge
                    success = false;
                    break;
                }
            }
            if(success)
                upstream_edges_.push_back(node);

            return(success);
        }

        bool remove_upstream_fx_node(fx_node_t *node)
        {
            bool success = false;
            for(unsigned i=0;i<upstream_edges_.size();i++)
            {
                if(upstream_edges_[i]==node)
                {
                    // found edge
                    upstream_edges_.erase(upstream_edges_.begin()+i);
                    success = true;
                    break;
                }
            }
            // return false if edge not found to remove
            return success;
        }

        bool is_fx_node_upstream(fx_node_t *node)
        {
#if CONSOLE_MIXER_DEBUG>1
            pic::logmsg() << "    checking upstream from " << chan_num_ << "with " << upstream_edges_.size() << " edges:";
#endif // CONSOLE_MIXER_DEBUG>1
            walk_flag_ = true;

            for(unsigned i=0; i<upstream_edges_.size(); i++)
            {
#if CONSOLE_MIXER_DEBUG>1
                pic::logmsg() << "        upstream edge " << upstream_edges_[i]->chan_num_;
#endif // CONSOLE_MIXER_DEBUG>1

                if(upstream_edges_[i]==node)
                {
#if CONSOLE_MIXER_DEBUG>1
                    pic::logmsg() << "            is upstream!";
#endif // CONSOLE_MIXER_DEBUG>1
                    return true;
                }
                if(node->walk_flag_)
                    continue;

                if(upstream_edges_[i]->is_fx_node_upstream(node))
                {
                    return true;
                }
            }

            return false;
        }

        std::vector<fx_node_t *> upstream_edges_;
        bool walk_flag_;
        unsigned chan_num_;
    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct console_master_controls_t: the master volume and pan
     *
     * This is a root for the volume and pan control wire. The clocksink is the main mixer.
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_master_controls_t: piw::root_t
    {
        consolemixer_master_controls_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc, float *lgain, float *rgain);
        ~consolemixer_master_controls_t() { invalidate(); }

        // invalidate(): disconnect, delete wires, break clock relationship and remove
        void invalidate();

        void set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan);
        // root_t inherited functions:
        // root_wire(): return a new wire from the root
        piw::wire_t *root_wire(const piw::event_data_source_t &es);
        // root_clock(): establish clock relationship of root_t with upstream root_ctl_t
        void root_clock();
        // root_opened(): set up root, establish clock relationship and latency
        void root_opened();
        // root_latency(): determine root latency
        void root_latency();
        // root_closed(): remove the root
        void root_closed();

        // process the root wire
        void process(unsigned long long t);

        // parent mixer implementation
        piw::consolemixer_t::impl_t *mixer_impl_;

        // the upstream (root_ctl_t) clock sink
        bct_clocksink_t *upstream_clk_;
        // the control wire
        consolemixer_controlwire_t *control_wire_;

    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_fx_chan_data_t: data in a channel about it's state and connection to an fx channel
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_fx_chan_data_t
    {
        bool enabled_;
        bool prefader_;
        bool clock_connected_;
        float sendgain_;
        consolemixer_sendwire_t *sendwire_;

    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_chan_t: a channel on the console mixer
     *
     * Behaves like a summer on the input (performs a polyphonic mix down).
     * The resulting buffer is used by the consolemixer to perform the final mix down
     * and the fx channels to perform the send mixes.
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_chan_t: piw::root_t
    {
        consolemixer_chan_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc,
                            piw::clockdomain_ctl_t *clk_domain, unsigned num, unsigned num_audio_chans,
                            const char *name = "consolemixer_chan_t");
        virtual ~consolemixer_chan_t()
        {
#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << ("~consolemixer_chan_t");
#endif // CONSOLE_MIXER_DEBUG>0
            invalidate();
        }

        // invalidate(): disconnect, delete wires, break clock relationship and remove
        void invalidate();

        void set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan);

        // root_t inherited functions:
        // root_wire(): return a new wire from the root
        piw::wire_t *root_wire(const piw::event_data_source_t &es);
        // root_clock(): establish clock relationship of root_t with upstream root_ctl_t
        void root_clock();
        // root_opened(): set up root, establish clock relationship and latency
        void root_opened();
        // root_latency(): determine root latency
        void root_latency();
        // root_closed(): remove the root
        void root_closed();

        // process call for input clock: sum the polyphonic inputs
        void input_process(unsigned long long from, unsigned long long t,unsigned bs);

        // perform the input wire mixing
        piw::data_nb_t mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float gain, unsigned long long t, unsigned bs);
        // apply a send level gain
        piw::data_nb_t apply_gain(piw::data_nb_t &buffer_in, float gain, unsigned long long t,unsigned bs);
        // add_to_sum_queue(): add wire buffers to the summer queue
        void add_to_sum_queue(unsigned c, const piw::data_nb_t &d);
        // resize_chans__(): fastcall to change number of channels
        static int resize_chans__(void *self_, void *c_);
        // set the number of channels, e.g. stereo, mono
        void set_channels(unsigned c);
        // add an fx chan
        void add_fx_channel(unsigned num);
        // remove an fx chan
        void remove_fx_channel(unsigned num);
        // set the enable to an fx chan and whether it has a clk connection (or will incur latency)
        void set_fx_send_enable(bool enable, bool has_clk_connection, unsigned fx_chan_num);
        //
        void set_fx_send_prefader(bool prefader, unsigned fx_chan_num);
        // get whether the send is enabled to an fx chan
        bool get_fx_send_enable(unsigned fx_chan_num);
        // get whether a clk connection is made to an fx chan
        bool get_fx_send_clk_connection(unsigned fx_chan_num);
        // add a clk connection to an fx chan downstream from this channel in the clk graph
        void add_fx_chan_downstream(consolemixer_fx_chan_t *fx_chan);
        // remove a clk connection to an fx chan downstream from this channel in the clk graph
        void remove_fx_chan_downstream(consolemixer_fx_chan_t *fx_chan);

        // input wires, one for each polyphonic input
        std::list<consolemixer_chan_audiowire_t *> input_wires_;
        // active wires, wires that events have started on
        pic::ilist_t<consolemixer_chan_audiowire_t> active_input_wires_;
        consolemixer_controlwire_t *control_wire_;

        // channel vector (e.g. stereo) with a list of signals to sum from the input wires
        pic::lckvector_t<pic::lcklist_t<piw::data_nb_t>::nbtype>::nbtype sum_mix_queue_;
        // audio buffer that inputs are summed down to
        pic::lckvector_t<piw::data_nb_t>::nbtype input_buffer_;

        // parent mixer implementation
        piw::consolemixer_t::impl_t *mixer_impl_;

        // the input clock sink
        consolemixer_input_clock_t *input_clk_;
        // the input clock sink state
        int input_clk_state_;
        // the clock domain
        piw::clockdomain_ctl_t *clk_domain_;
        // the upstream (root_ctl_t) clock sink
        bct_clocksink_t *upstream_clk_;
        // the object id (for debug)
        std::string id_;
        // the input clock id (for debug)
        std::string input_clk_id_;

        // map of fx channel data
        std::map<unsigned, consolemixer_fx_chan_data_t> fx_chans_;

        // left and right channel gains
        float lgain_, rgain_;

        // number of audio channels
        unsigned num_audio_chans_;
        // the id number of the channel
        unsigned num_;

        // volume control level to gain function
        pic::f2f_t volfunc_;

        // signal mask
        unsigned long long sigmask_;

    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_fx_chan_t: an effect send-return channel on the console mixer
     *
     * As the mixer channel, but adds a send output to form an effect send and return.
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_fx_chan_t: consolemixer_chan_t, fx_node_t, piw::wire_ctl_t, piw::root_ctl_t, piw::event_data_source_real_t
    {
        consolemixer_fx_chan_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc,
                                piw::clockdomain_ctl_t *clk_domain, const piw::cookie_t &output_cookie,
                                unsigned num, unsigned num_audio_chans);
        ~consolemixer_fx_chan_t()
        {
#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << "~consolemixer_fx_chan_t";
#endif // CONSOLE_MIXER_DEBUG>0
            invalidate();
        }
        // invalidate(): disconnect, delete wires, break clock relationship and remove
        void invalidate();

        // process call for output clock: perform send mix down
        void output_process(unsigned long long from, unsigned long long t, unsigned bs);

        // perform the send mixing
        piw::data_nb_t send_mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, unsigned long long t, unsigned bs);
        // add channel buffers to the send mix queue
        void add_to_send_queue(unsigned c, const piw::data_nb_t &d);

        // the output clock sink
        consolemixer_output_clock_t *output_clk_;
        // the output clock state
        int output_clk_state_;
        // the downstream (root_t) clock sink
        bct_clocksink_t *downstream_clk_;
        // the output clock id (for debug)
        std::string output_clk_id_;

        // queue of channels to mix
        pic::lckvector_t<pic::lcklist_t<piw::data_nb_t>::nbtype>::nbtype send_mix_queue_;

        // output audio buffer
        piw::xevent_data_buffer_t output_buffer_;

    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_input_clock_t: clock sink for the input of a channel
     *
     * For an fx channel, this is the return input.
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_input_clock_t: piw::clocksink_t
    {
        consolemixer_input_clock_t(consolemixer_chan_t *chan) : chan_(chan) {}
        ~consolemixer_input_clock_t() {}

        void clocksink_ticked(unsigned long long from, unsigned long long t)
        {
            // tick the input clock of the console mixer input channel
            chan_->input_process(from, t, get_buffer_size());
        }
        consolemixer_chan_t *chan_;
    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_output_clock_t: clock sink for the output of a channel
     *
     * For an fx channel, this is the send output.
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_output_clock_t: piw::clocksink_t
    {
        consolemixer_output_clock_t(consolemixer_fx_chan_t *chan) : chan_(chan) {}
        ~consolemixer_output_clock_t() {}

        void clocksink_ticked(unsigned long long from, unsigned long long t)
        {
            // tick the output clock of the console mixer output channel
            chan_->output_process(from, t, get_buffer_size());
        }
        consolemixer_fx_chan_t *chan_;
    };

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_chan_audiowire_t: mixer channel input wire
     *
     * Inherits from element_t, so the active list of wires in the console mixer channel can be an ilist.
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_chan_audiowire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        consolemixer_chan_audiowire_t(consolemixer_chan_t *chan, const piw::event_data_source_t &es);
        ~consolemixer_chan_audiowire_t() { invalidate(); }

        // wire_t virtual functions
        void wire_closed() { delete this; }

        // unsubscribe, disconnect and remove the wire
        void invalidate();

        // event_data_sink_t virtual functions:
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void event_start(unsigned s, const piw::data_nb_t &d, const piw::xevent_data_buffer_t &ed);
        bool event_end(unsigned long long t);

        // called when the each of the wires in the queue is ticked from mixer clocksink_ticked
        void ticked(unsigned long long t);

        // the parent channel
        consolemixer_chan_t *chan_;
        // iterator for signals in the input event
        piw::xevent_data_buffer_t::iter_t iterator_;

    };

    consolemixer_chan_audiowire_t::consolemixer_chan_audiowire_t(consolemixer_chan_t *chan, const piw::event_data_source_t &es) : chan_(chan)
    {
        // add this wire to list in chan
        chan_->input_wires_.push_back(this);
        // subscribe to the event data sink
        subscribe(es);
    }

    void consolemixer_chan_audiowire_t::invalidate()
    {
        // unsubscribe from event data sink
        unsubscribe();
        // disconnect this wire
        wire_t::disconnect();

        // remove from wire list in chan
        chan_->input_wires_.remove(this);
    }

    void consolemixer_chan_audiowire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
    {
        iterator_->set_signal(s,n);
        iterator_->reset(s,t);
    }

    void consolemixer_chan_audiowire_t::event_start(unsigned s, const piw::data_nb_t &d, const piw::xevent_data_buffer_t &ed)
    {
#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << "consolemixer_chan_audiowire_t::event_start";
#endif // CONSOLE_MIXER_DEBUG>1

        piw::data_nb_t d2;
        iterator_ = ed.iterator();
        // start of event, so reset buffers
        for(unsigned c=0; c<chan_->num_audio_chans_; c++)
        {
            iterator_->reset(c+1, d.time());
        }
        // add to active list in chan
        chan_->active_input_wires_.append(this);

        // ensure channel clock is started up to handle new event
        if(chan_->input_clk_state_ == CLKSTATE_IDLE)
        {
            chan_->input_clk_->tick_suppress(false);
            chan_->input_clk_state_ = CLKSTATE_STARTUP;
        }

    }

    bool consolemixer_chan_audiowire_t::event_end(unsigned long long t)
    {
#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << "consolemixer_chan_audiowire_t::event_end";
#endif // CONSOLE_MIXER_DEBUG>1

        ticked(t);
        iterator_.clear();
        // remove this wire (element) from the active ilist
        remove();
        return true;
    }

    void consolemixer_chan_audiowire_t::ticked(unsigned long long t)
    {
        for(unsigned c=0; c<chan_->num_audio_chans_; c++)
        {
            piw::data_nb_t d;
            // next signal up to time t, i.e. the next block of audio (should be always 1 block)
            while(iterator_->nextsig(c+1, d, t))
                chan_->add_to_sum_queue(c, d);
        }
    }

    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_controlwire_t: audio controls wire
     *
     * Contains volume and pan level signals, which are mapped to gains in the parent class.
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_controlwire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t
    {
        consolemixer_controlwire_t(const pic::f2f_t &volfunc, const pic::f2f_t &panfunc, float *lgain, float *rgain);
        ~consolemixer_controlwire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void process(unsigned long long t);
        void set_gains();
        void set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan);

        piw::dataqueue_t vinput_, pinput_;
        unsigned long long vindex_, pindex_;

        // volume control signal and left and right pan control component signals
        float volctl_, lpanctl_, rpanctl_;

        // pointers to parent class left and right gains to set from control levels
        float *lgain_, *rgain_;

        // volume and pan control level to gain functions
        pic::flipflop_functor_t<pic::f2f_t> volfunc_;
        pic::flipflop_functor_t<pic::f2f_t> panfunc_;

        bool reset_pan_;

    };

    consolemixer_controlwire_t::consolemixer_controlwire_t(const pic::f2f_t &volfunc, const pic::f2f_t &panfunc, float *lgain, float *rgain):
        volctl_(100), lpanctl_(0), rpanctl_(0), lgain_(lgain), rgain_(rgain), volfunc_(volfunc), panfunc_(panfunc), reset_pan_(false)
    {
    }

    void consolemixer_controlwire_t::invalidate()
    {
        // unsubscribe from event data sink
        unsubscribe();
        // disconnect this wire
        wire_t::disconnect();

    }

    void consolemixer_controlwire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
    {
        piw::data_nb_t d2;

        if(s==1)
        {
            vinput_=n;
            vindex_=0;
            vinput_.latest(d2,&vindex_,t);
        }

        if(s==2)
        {
            pinput_=n;
            pindex_=0;
            pinput_.latest(d2,&pindex_,t);
        }

    }

    void consolemixer_controlwire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei)
    {
        // assign signals to dataqueues
        vinput_=ei.signal(1);
        pinput_=ei.signal(2);
        // reset indices on event start
        vindex_=0;
        pindex_=0;

        process(id.time());
    }

    void consolemixer_controlwire_t::process(unsigned long long t)
    {
        piw::data_nb_t d;
        unsigned long long nvindex, npindex;

        if(vinput_.latest(d,&nvindex,t) && nvindex!=vindex_)
        {
            vindex_=nvindex;
            volctl_ = d.as_renorm(-70,14,0);
            reset_pan_ = true;
        }

        if(pinput_.latest(d,&npindex,t) && npindex!=pindex_)
        {
            pindex_=npindex;
            float u = d.as_array_ubound();
            float l = d.as_array_lbound();
            float r = d.as_array_rest();

            lpanctl_ = piw::denormalise(u,l,r,-d.as_norm());
            rpanctl_ = piw::denormalise(u,l,r,d.as_norm());

            reset_pan_ = true;
        }

        if(reset_pan_)
        {
            reset_pan_ = false;
            set_gains();
        }
    }

    void consolemixer_controlwire_t::set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan)
    {
        panfunc_ = pan;
        volfunc_ = vol;
        reset_pan_ = true;
    }

    void consolemixer_controlwire_t::set_gains()
    {
        float vol = volfunc_(volctl_);

        *lgain_=vol * panfunc_(lpanctl_);
        *rgain_=vol * panfunc_(rpanctl_);

#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << "consolemixer_controlwire_t::set_gains() lgain="<< lgain_ <<" *lgain=" << *lgain_ << " rgain=" << rgain_ << " *rgain=" << *rgain_;
#endif // CONSOLE_MIXER_DEBUG>1
    }

    bool consolemixer_controlwire_t::event_end(unsigned long long t)
    {
        return true;
    }


    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * struct consolemixer_sendwire_t: send level control wire
     *
     * Contains a send level signal, which is mapped to a gain in the parent class. Each channel has a send wire for every effect channel in the mixer.
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    struct consolemixer_sendwire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t
    {
        consolemixer_sendwire_t(consolemixer_chan_t *chan, const piw::event_data_source_t &es, const pic::f2f_t &volfunc, float *sendgain);
        ~consolemixer_sendwire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void process(unsigned long long t);

        piw::stereomixer_t::impl_t *impl_;
        piw::dataqueue_t sendinput_;
        unsigned long long sendindex_;

        // the parent channel
        consolemixer_chan_t *chan_;

        // send control signal
        float sendctl_;

        // pointers to parent class gain to set from control level
        float *sendgain_;

        // volume control level to gain functions
        pic::flipflop_functor_t<pic::f2f_t> volfunc_;

    };

    consolemixer_sendwire_t::consolemixer_sendwire_t(consolemixer_chan_t *chan, const piw::event_data_source_t &es, const pic::f2f_t &volfunc, float *sendgain):
        chan_(chan), sendctl_(100), sendgain_(sendgain), volfunc_(volfunc)
    {
        // subscribe to the event data sink
        subscribe(es);
    }

    void consolemixer_sendwire_t::invalidate()
    {
        // unsubscribe from event data sink
        unsubscribe();
        // disconnect this wire
        wire_t::disconnect();
    }

    void consolemixer_sendwire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
    {
        if(s==1)
        {
            piw::data_nb_t d2;
            sendinput_=n;
            sendindex_=0;
            sendinput_.latest(d2,&sendindex_,t);
        }
    }

    void consolemixer_sendwire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei)
    {
        // assign signals to dataqueues
        sendinput_=ei.signal(1);
        // reset indices on event start
        sendindex_=0;

        process(id.time());

    }

    void consolemixer_sendwire_t::process(unsigned long long t)
    {
        piw::data_nb_t d;
        if(sendinput_.latest(d,&sendindex_,t))
        {
            sendindex_++;
            sendctl_ = d.as_renorm(-70,14,0);

            *sendgain_ = volfunc_(sendctl_);

#if CONSOLE_MIXER_DEBUG>1
            pic::logmsg() << "consolemixer_sendwire_t::process() sendgain=" << *sendgain_;
#endif // CONSOLE_MIXER_DEBUG>1

        }

    }

    bool consolemixer_sendwire_t::event_end(unsigned long long t)
    {
        return true;
    }



    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * consolemixer_master_controls_t struct methods
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    consolemixer_master_controls_t::consolemixer_master_controls_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc, float *lgain, float *rgain) :
        root_t(0), mixer_impl_(mixer_impl), upstream_clk_(0)
    {
        control_wire_ = new consolemixer_controlwire_t(volfunc, panfunc, lgain, rgain);

    }

    void consolemixer_master_controls_t::set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan)
    {
        control_wire_->set_curves(vol,pan);
    }

    void consolemixer_master_controls_t::invalidate()
    {
        // disconnect the root from upstream root_ctl_t
        disconnect();

        // remove clock of upstream root_ctl_t
        if(upstream_clk_)
        {
            // the clocksink is the main mixer clocksink
            mixer_impl_->remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }

        delete control_wire_;

    }

    piw::wire_t *consolemixer_master_controls_t::root_wire(const piw::event_data_source_t &es)
    {
        control_wire_->subscribe(es);
        return control_wire_;
    }

    void consolemixer_master_controls_t::root_clock()
    {
        // get the root_ctl_t clock of root_t
        bct_clocksink_t *s(get_clock());

        // if old upstream clock then remove
        if(upstream_clk_)
        {
            mixer_impl_->remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }
        // save clock of upstream root_ctl_t
        if(s)
        {
            upstream_clk_ = s;
            mixer_impl_->add_upstream(s);
#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << "consolemixer_master_controls_t::root_wire add upstream " << s;
#endif // CONSOLE_MIXER_DEBUG>0
        }
    }

    void consolemixer_master_controls_t::root_opened()
    {
        root_clock();
        root_latency();
    }

    void consolemixer_master_controls_t::root_latency()
    {
        mixer_impl_->set_sink_latency(get_latency());
    }

    void consolemixer_master_controls_t::root_closed() {}

    void consolemixer_master_controls_t::process(unsigned long long t)
    {
        control_wire_->process(t);
    }



    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * consolemixer_chan_t struct methods
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */

    consolemixer_chan_t::consolemixer_chan_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc,
                                                piw::clockdomain_ctl_t *clk_domain, unsigned num, unsigned num_audio_chans,
                                                const char *name):
        root_t(0), mixer_impl_(mixer_impl), input_clk_state_(CLKSTATE_IDLE), clk_domain_(clk_domain),
        upstream_clk_(0), lgain_(1), rgain_(1), num_audio_chans_(num_audio_chans), num_(num), volfunc_(volfunc)
    {
        sum_mix_queue_.resize(num_audio_chans);
        input_buffer_.resize(num_audio_chans);

        sigmask_=(1ULL<<num_audio_chans)-1;

        input_clk_ = new consolemixer_input_clock_t(this);

        std::stringstream out;
        // make class string id
        out << name << "_" << num;
        id_ = out.str();
        // make input clk string id
        out << "_input_clk";
        input_clk_id_ = out.str();

        clk_domain->sink(input_clk_,input_clk_id_.c_str());

        control_wire_ = new consolemixer_controlwire_t(volfunc, panfunc, &lgain_, &rgain_);

        // enable input clock but suppress it
        input_clk_->tick_enable(true);
    }

    void consolemixer_chan_t::set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan)
    {
        volfunc_ = vol;
        control_wire_->set_curves(vol,pan);
    }

    void consolemixer_chan_t::invalidate()
    {
#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << "consolemixer_chan_t::invalidate()";
#endif // CONSOLE_MIXER_DEBUG>1
        std::list<consolemixer_chan_audiowire_t *>::iterator i;

        // disconnect the root from upstream root_ctl_t
        disconnect();

        // destruct all inputs - invalidates them also
        while((i=input_wires_.begin())!=input_wires_.end())
        {
            delete (*i);
        }

        // remove clock of upstream root_ctl_t
        if(upstream_clk_)
        {
            input_clk_->remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }

        delete control_wire_;
        delete input_clk_;

    }

    piw::wire_t *consolemixer_chan_t::root_wire(const piw::event_data_source_t &es)
    {

        piw::data_t path(es.path());
        if(path.as_pathlen()>0)
        {
            unsigned aggregator_output = path.as_path()[0];

            if(path.as_path()[0]==1)
            {
#if CONSOLE_MIXER_DEBUG>1
                pic::logmsg() << id_ << "::root_wire adding control wire, path=" << aggregator_output;
#endif // CONSOLE_MIXER_DEBUG>1
                control_wire_->subscribe(es);
                return control_wire_;
            }

            if(path.as_path()[0]==2)
            {
#if CONSOLE_MIXER_DEBUG>1
                pic::logmsg() << id_ << "::root_wire adding audio wire, path=" << aggregator_output;
#endif // CONSOLE_MIXER_DEBUG>1
                // create new audio wire - will add itself to the channel wire list
                consolemixer_chan_audiowire_t *audio_wire = new consolemixer_chan_audiowire_t(this, es);
                return audio_wire;
            }

            if(path.as_path()[0]>2)
            {
#if CONSOLE_MIXER_DEBUG>1
                pic::logmsg() << id_ << "::root_wire adding send wire, path=" << aggregator_output;
#endif // CONSOLE_MIXER_DEBUG>1
                unsigned fx_chan_num = aggregator_output-3;
                float *sendgain = &fx_chans_[fx_chan_num].sendgain_;
                // create new send level wire - will add itself to the send wire list
                consolemixer_sendwire_t *send_wire = new consolemixer_sendwire_t(this, es, volfunc_, sendgain);
                fx_chans_[fx_chan_num].sendwire_ = send_wire;
                return send_wire;
            }


        }

        return 0;

    }

    void consolemixer_chan_t::root_clock()
    {
        // get the root_ctl_t clock of root_t
        bct_clocksink_t *s(get_clock());

        // if old upstream clock then remove
        if(upstream_clk_)
        {
            input_clk_->remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }
        // save clock of upstream root_ctl_t
        if(s)
        {
            upstream_clk_ = s;
            input_clk_->add_upstream(s);
#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << id_<<"::root_clock add upstream " << s;
#endif // CONSOLE_MIXER_DEBUG>0
        }
    }

    void consolemixer_chan_t::root_opened()
    {
        root_clock();
        root_latency();
    }

    void consolemixer_chan_t::root_latency()
    {
        input_clk_->set_sink_latency(get_latency());
    }

    void consolemixer_chan_t::root_closed()
    {
    }

    void consolemixer_chan_t::input_process(unsigned long long from, unsigned long long t, unsigned bs)
    {
        // if no active wires then shut down
        consolemixer_chan_audiowire_t *w=active_input_wires_.head();
        if(!w)
        {
            input_clk_state_=CLKSTATE_SHUTDOWN;
        }

        // tick all the active wires
        while(w)
        {
            w->ticked(t);
            w=active_input_wires_.next(w);
        }

        control_wire_->process(t);

        piw::data_nb_t left_audio, right_audio, left_audio_gain, right_audio_gain, send_audio;
        std::map<unsigned, consolemixer_fx_chan_data_t>::iterator i;

        // TODO: support more audio channels?
        // mix left channel
        left_audio = mix(sum_mix_queue_[0], 1.0f, t, bs);
        // mix right channel
        right_audio = mix(sum_mix_queue_[1], 1.0f, t, bs);

        // apply left fader gain
        left_audio_gain = apply_gain(left_audio, lgain_, t, bs);
        // apply right fader gain
        right_audio_gain = apply_gain(right_audio, rgain_, t, bs);
        
        // add to final mix queue
        mixer_impl_->add_to_mix_queues(0, left_audio_gain);
        // add to final mix queue
        mixer_impl_->add_to_mix_queues(1, right_audio_gain);

        // add sum to fx send queues
        for (i=fx_chans_.begin(); i!=fx_chans_.end(); ++i)
        {
            if(i->second.enabled_)
            {
                // get the lastest gain from the send level wire
                i->second.sendwire_->process(t);

				// add send buffers if gain is effectively non-zero
                if(i->second.sendgain_>1.0e-6f)
                {
                    // apply to audio buffer
                    if(i->second.prefader_)
                        send_audio = apply_gain(left_audio, i->second.sendgain_, t, bs);
                    else
                        send_audio = apply_gain(left_audio_gain, i->second.sendgain_, t, bs);
                    mixer_impl_->add_to_fx_send_queues(i->first, 0, send_audio);

                    if(i->second.prefader_)
                        send_audio = apply_gain(right_audio, i->second.sendgain_, t, bs);
                    else
                        send_audio = apply_gain(right_audio_gain, i->second.sendgain_, t, bs);
                    mixer_impl_->add_to_fx_send_queues(i->first, 1, send_audio);
                }

            }
        }


        if(input_clk_state_==CLKSTATE_STARTUP)
        {
            input_clk_state_=CLKSTATE_RUNNING;

#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << id_ <<" input clk running";
#endif // CONSOLE_MIXER_DEBUG>1
        }

        if(input_clk_state_==CLKSTATE_SHUTDOWN)
        {
            input_clk_state_ = CLKSTATE_IDLE;
            input_clk_->tick_suppress(true);

#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << id_ <<" input clk idle";
#endif // CONSOLE_MIXER_DEBUG>1
        }
    }

    piw::data_nb_t consolemixer_chan_t::mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float gain, unsigned long long t, unsigned bs)
    {
        float *f,*fs;
        piw::data_nb_t d = piw::makenorm_nb(t,bs,&f,&fs);
        memset(f,0,bs*sizeof(float));

        while(queue.size()>1)
        {
            const float *df = queue.front().as_array();
            unsigned dfl = std::min((unsigned)queue.front().as_arraylen(),bs);
            pic::vector::vectadd(df,1,f,1,f,1,dfl);
            queue.pop_front();
        }

        if(queue.size()==1)
        {
            const float *df = queue.front().as_array();
            unsigned dfl = std::min((unsigned)queue.front().as_arraylen(),bs);
            // f = (df + f)*gain
            pic::vector::vectasm(df,1,f,1,&gain,f,1,dfl);
            queue.pop_front();
        }

        *fs=f[bs-1];
        return d;
    }

    piw::data_nb_t consolemixer_chan_t::apply_gain(piw::data_nb_t &buffer_in, float gain, unsigned long long t, unsigned bs)
    {
        float *f,*fs;
        piw::data_nb_t buffer_out = piw::makenorm_nb(t,bs,&f,&fs);
        memset(f,0,bs*sizeof(float));

        const float *buffer_in_float = buffer_in.as_array();
        unsigned buffer_in_len = std::min(buffer_in.as_arraylen(),bs);
        // f = (buffer_in_float + f)*gain
        pic::vector::vectasm(buffer_in_float,1,f,1,&gain,f,1,buffer_in_len);

        *fs=f[bs-1];
        return buffer_out;
    }

    void consolemixer_chan_t::add_to_sum_queue(unsigned c, const piw::data_nb_t &d)
    {
        // add a buffer pointer to the queue each time a wire has data
        if(c<num_audio_chans_)
        {
            sum_mix_queue_[c].push_back(d);
            if(input_clk_state_ == CLKSTATE_SHUTDOWN)
                input_clk_state_ = CLKSTATE_RUNNING;
        }
    }

    int consolemixer_chan_t::resize_chans__(void *self_, void *c_)
    {
        consolemixer_chan_t *self = (consolemixer_chan_t *)self_;
        unsigned c = *(unsigned *)c_;

        // grow number of channels
        while(c>self->num_audio_chans_)
        {
            self->num_audio_chans_++;
            self->sum_mix_queue_.resize(self->num_audio_chans_);
            self->input_buffer_.resize(self->num_audio_chans_);
        }

        // shrink number of channels
        while(c<self->num_audio_chans_)
        {
            self->num_audio_chans_--;
            self->sum_mix_queue_.resize(self->num_audio_chans_);
            self->input_buffer_.resize(self->num_audio_chans_);
        }

        self->sigmask_=(1ULL<<c)-1;

        return 0;
    }

    void consolemixer_chan_t::set_channels(unsigned c)
    {
        piw::tsd_fastcall(consolemixer_chan_t::resize_chans__,this,&c);
    }

    void consolemixer_chan_t::add_fx_channel(unsigned fx_chan_num)
    {
        consolemixer_fx_chan_data_t fx_chan_data = {false, false, 1};
        fx_chans_.insert(std::make_pair(fx_chan_num, fx_chan_data));
    }

    void consolemixer_chan_t::remove_fx_channel(unsigned fx_chan_num)
    {
        fx_chans_.erase(fx_chan_num);
    }

    void consolemixer_chan_t::set_fx_send_enable(bool enable, bool has_clk_connection, unsigned fx_chan_num)
    {
        fx_chans_[fx_chan_num].enabled_ = enable;
        fx_chans_[fx_chan_num].clock_connected_ = has_clk_connection;
    }

    void consolemixer_chan_t::set_fx_send_prefader(bool prefader, unsigned fx_chan_num)
    {
        fx_chans_[fx_chan_num].prefader_ = prefader;
    }
    
    bool consolemixer_chan_t::get_fx_send_enable(unsigned fx_chan_num)
    {
        return fx_chans_[fx_chan_num].enabled_;
    }

    bool consolemixer_chan_t::get_fx_send_clk_connection(unsigned fx_chan_num)
    {
        return fx_chans_[fx_chan_num].clock_connected_;
    }

    void consolemixer_chan_t::add_fx_chan_downstream(consolemixer_fx_chan_t *fx_chan)
    {
        // make clock relationship: this chan input clk -> fx_chan output clock
        fx_chan->output_clk_->add_upstream(this->input_clk_);

#if CONSOLE_MIXER_DEBUG>0
        pic::logmsg() << id_ << "::add_chan_downstream " << num_ << "->" << fx_chan->num_;
#endif // CONSOLE_MIXER_DEBUG>0
    }

    void consolemixer_chan_t::remove_fx_chan_downstream(consolemixer_fx_chan_t *fx_chan)
    {
        // remove clock relationship: this chan input clk -> fx_chan output clock
        fx_chan->output_clk_->remove_upstream(this->input_clk_);

#if CONSOLE_MIXER_DEBUG>0
        pic::logmsg() << id_ << "::remove_chan_downstream " << num_ << "->" << fx_chan->num_;
#endif // CONSOLE_MIXER_DEBUG>0
    }


    /* --------------------------------------------------------------------------------------------------------------------------------------------------
     * consolemixer_fx_chan_t struct methods
     *
     * --------------------------------------------------------------------------------------------------------------------------------------------------
     */
    consolemixer_fx_chan_t::consolemixer_fx_chan_t(piw::consolemixer_t::impl_t *mixer_impl, const pic::f2f_t &volfunc, const pic::f2f_t &panfunc,
                                                    piw::clockdomain_ctl_t *clk_domain, const piw::cookie_t &output_cookie,
                                                    unsigned num, unsigned num_audio_chans) :
    consolemixer_chan_t(mixer_impl, volfunc, panfunc, clk_domain, num, num_audio_chans, "consolemixer_fx_chan_t"),
    fx_node_t(num), piw::event_data_source_real_t(piw::pathnull(0)),
    output_clk_state_(CLKSTATE_IDLE), downstream_clk_(0), output_buffer_(3,PIW_DATAQUEUE_SIZE_ISO)
    {
        send_mix_queue_.resize(num_audio_chans_);

        output_clk_ = new consolemixer_output_clock_t(this);


        std::stringstream out;
        std::string output_clk_id;
        // make class string id
        out << "consolemixer_fx_chan_t_" << num;
        id_ = out.str();
        // make output clk string id
        out << "_output_clk";
        output_clk_id_ = out.str();

        clk_domain->sink(output_clk_,output_clk_id_.c_str());


        // pass the output clock clocksink_t through the root_ctl_t to the downstream root_t
        // causes root_clock() to be called to establish the clock relationship
        set_clock(output_clk_);

        // connect the output cookie, have to do this before connecting the wire!
        connect(output_cookie);

        // connect this root_ctl_t to this event_data_source_real_t
        connect_wire(this, source());

        // enable input clock but suppress it
        output_clk_->tick_enable(true);

    }

    void consolemixer_fx_chan_t::invalidate()
    {
#if CONSOLE_MIXER_DEBUG>1
        pic::logmsg() << "consolemixer_fx_chan_t::invalidate()";
#endif // CONSOLE_MIXER_DEBUG>1

        // shutdown the output event_data_source_t
        source_shutdown();

        delete output_clk_;

    }

    piw::data_nb_t consolemixer_fx_chan_t::send_mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, unsigned long long t, unsigned bs)
    {
        float *f,*fs;
        piw::data_nb_t d = piw::makenorm_nb(t,bs,&f,&fs);
        memset(f,0,bs*sizeof(float));

        while(queue.size())
        {
            const float *df = queue.front().as_array();
            unsigned dfl = std::min(queue.front().as_arraylen(),bs);
            pic::vector::vectadd(df,1,f,1,f,1,dfl);
            queue.pop_front();
        }

        *fs=f[bs-1];
        return d;
    }

    void consolemixer_fx_chan_t::add_to_send_queue(unsigned c, const piw::data_nb_t &d)
    {
        // add a buffer pointer to the queue each time a wire has data
        if(c<num_audio_chans_)
        {
            send_mix_queue_[c].push_back(d);
            if(output_clk_state_ == CLKSTATE_IDLE)
            {
                output_clk_state_ = CLKSTATE_STARTUP;
                output_clk_->tick_suppress(false);
            }
        }
    }

    void consolemixer_fx_chan_t::output_process(unsigned long long from, unsigned long long t, unsigned bs)
    {

        bool queues_empty_ = true;

        // are there any buffers to mix?
        for(unsigned c=0; c<num_audio_chans_; c++)
            if(send_mix_queue_[c].size()!=0)
                queues_empty_ = false;

        // nothing in queue to mix
        if(queues_empty_)
        {
            output_clk_state_=CLKSTATE_SHUTDOWN;
        }
        else
        {
#if CONSOLE_MIXER_DEBUG>1
            pic::logmsg() << id_ << " queue size " << send_mix_queue_[0].size();
#endif // CONSOLE_MIXER_DEBUG>1

            // perform mixing and write to buffer
            for(unsigned c=0; c<num_audio_chans_; c++)
            {
                piw::data_nb_t d = send_mix(send_mix_queue_[c],t,bs);
                output_buffer_.add_value(c+1,d);
            }
        }

        // manage start up and shut down of clock
        if(output_clk_state_==CLKSTATE_STARTUP)
        {
            output_clk_state_=CLKSTATE_RUNNING;
            // start the data source with the buffer
            source_start(0, piw::pathnull_nb(t), output_buffer_);

#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << id_ << " output clk running";
#endif // CONSOLE_MIXER_DEBUG>0
        }

        if(output_clk_state_==CLKSTATE_SHUTDOWN)
        {
            source_end(t);
            output_buffer_ = piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_ISO);
            output_clk_->tick_suppress(true);

            output_clk_state_=CLKSTATE_IDLE;

#if CONSOLE_MIXER_DEBUG>0
            pic::logmsg() << id_ << " output clk idle";
#endif // CONSOLE_MIXER_DEBUG>0
        }
    }

};  // namespace



/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * consolemixer_t::impl_t struct methods
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

piw::consolemixer_t::impl_t::impl_t(const pic::f2f_t &vol, const pic::f2f_t &pan,
                                    piw::clockdomain_ctl_t *clk_domain, const piw::cookie_t &c)
        : piw::event_data_source_real_t(piw::pathnull(0)), clk_state_(CLKSTATE_IDLE), clk_domain_(clk_domain),
        output_buffer_(3,PIW_DATAQUEUE_SIZE_ISO), lgain_(1), rgain_(1), volfunc_(vol), panfunc_(pan)
{
    // TODO: stereo mixer, handle different number of channels?
    num_audio_chans_ = 2;
    chan_mix_queue_.resize(num_audio_chans_);

    // add to the clock domain
    clk_domain->sink(this, "consolemixer");

    // pass this clocksink_t through the root_ctl_t to the downstream root_t
    // causes root_clock() to be called to establish the clock relationship
    set_clock(this);

    // connect the output cookie
    connect(c);

    // connect this root_ctl_t to this event_data_source_real_t
    connect_wire(this, source());

    // new root_t to receive volume and pan signals along a control wire
    master_controls_ = new consolemixer_master_controls_t(this, volfunc_, panfunc_, &lgain_, &rgain_);

    // enable ticking but suppress it
    tick_enable(true);

}

void piw::consolemixer_t::impl_t::invalidate()
{
    std::map<unsigned, consolemixer_chan_t *>::iterator i;
    std::map<unsigned, consolemixer_fx_chan_t *>::iterator j;

    // shutdown the output event_data_source_t
    source_shutdown();

    // destruct all channels
    while((i=channels_.begin())!=channels_.end())
    {
        delete i->second;
        channels_.erase(i);
    }

    while((j=fx_channels_.begin())!=fx_channels_.end())
    {
        delete j->second;
        fx_channels_.erase(j);
    }

    delete master_controls_;

}

void piw::consolemixer_t::impl_t::clocksink_ticked(unsigned long long from, unsigned long long t)
{
    // main process call

    bool queues_empty_ = true;
    unsigned bs = get_buffer_size();

    // are there any buffers to mix?
    for(unsigned c=0; c<num_audio_chans_; c++)
        if(chan_mix_queue_[c].size()!=0)
            queues_empty_ = false;

    // nothing in queue to mix
    if(queues_empty_)
    {
        clk_state_=CLKSTATE_SHUTDOWN;
    }
    else
    {
#if CONSOLE_MIXER_DEBUG>1
//        pic::logmsg() << "consolemixer_t::impl_t mix queue size " << chan_mix_queue_[0].size();
#endif // CONSOLE_MIXER_DEBUG>1

        // process master controls
        master_controls_->process(t);

        // perform mixing and write to buffer
        // TODO: support more channels than stereo?
        output_buffer_.add_value(1,mix(chan_mix_queue_[0],lgain_,t,bs));
        output_buffer_.add_value(2,mix(chan_mix_queue_[1],rgain_,t,bs));

    }

    // manage start up and shut down of clock
    if(clk_state_==CLKSTATE_STARTUP)
    {
        clk_state_=CLKSTATE_RUNNING;
        // start the data source with the buffer
        source_start(0, piw::pathnull_nb(t), output_buffer_);

#if CONSOLE_MIXER_DEBUG>0
        pic::logmsg() << "consolemixer_t::impl_t clk running";
#endif // CONSOLE_MIXER_DEBUG>0
    }

    if(clk_state_==CLKSTATE_SHUTDOWN)
    {
        source_end(t);
        output_buffer_ = piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_ISO);
        tick_suppress(true);

        clk_state_=CLKSTATE_IDLE;

#if CONSOLE_MIXER_DEBUG>0
        pic::logmsg() << "consolemixer_t::impl_t clk idle";
#endif // CONSOLE_MIXER_DEBUG>0
    }


}

piw::data_nb_t piw::consolemixer_t::impl_t::mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float gain, unsigned long long t,unsigned bs)
{
    float *f,*fs;
    piw::data_nb_t d = piw::makenorm_nb(t,bs,&f,&fs);
    memset(f,0,bs*sizeof(float));

    while(queue.size()>1)
    {
        const float *df = queue.front().as_array();
        unsigned dfl = std::min((unsigned)queue.front().as_arraylen(),bs);
        pic::vector::vectadd(df,1,f,1,f,1,dfl);
        queue.pop_front();
    }

    if(queue.size()==1)
    {
        const float *df = queue.front().as_array();
        unsigned dfl = std::min((unsigned)queue.front().as_arraylen(),bs);
        pic::vector::vectasm(df,1,f,1,&gain,f,1,dfl);
        queue.pop_front();
    }

    *fs=f[bs-1];
    return d;
}

void piw::consolemixer_t::impl_t::add_to_mix_queues(unsigned audio_channel, const piw::data_nb_t &buffer)
{
    // add a buffer pointer to the queue each time a wire has data
    if(audio_channel<num_audio_chans_)
    {
        // add to final mix queue
        chan_mix_queue_[audio_channel].push_back(buffer);
        if(clk_state_ == CLKSTATE_IDLE)
        {
            clk_state_ = CLKSTATE_STARTUP;
            tick_suppress(false);
        }

    }
}

void piw::consolemixer_t::impl_t::add_to_fx_send_queues(unsigned fx_chan_num, unsigned audio_channel, const piw::data_nb_t &buffer)
{
    fx_channels_[fx_chan_num]->add_to_send_queue(audio_channel, buffer);
}

piw::cookie_t piw::consolemixer_t::impl_t::create_channel(unsigned num)
{
    std::map<unsigned, consolemixer_chan_t *>::iterator i;

    if((i=channels_.find(num))!=channels_.end())
    {
        PIC_THROW("console mixer: channel in use");
    }

    // TODO: create channels with different number of audio channels? stereo for now
    consolemixer_chan_t *chan = new consolemixer_chan_t(this, volfunc_, panfunc_, clk_domain_, num, 2);

    // make the channel upstream of the main mixdown
    // to ensure each channel mixdown happens first
    add_upstream(chan->input_clk_);

    // add to channel dictionary
    channels_.insert(std::make_pair(num, chan));

    return cookie_t(chan);
}

void piw::consolemixer_t::impl_t::remove_channel(unsigned num)
{
    std::map<unsigned, consolemixer_chan_t *>::iterator i;

    if((i=channels_.find(num))!=channels_.end())
    {
        // actually delete channel
        delete i->second;
        // now remove from map
        channels_.erase(i);
        update_latency();
    }

}

void piw::consolemixer_t::impl_t::set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan)
{
    volfunc_ = vol;
    panfunc_ = pan;

    std::map<unsigned, consolemixer_chan_t *>::iterator ci;

    for(ci=channels_.begin(); ci!=channels_.end(); ci++)
    {
        ci->second->set_curves(vol,pan);
    }

    std::map<unsigned, consolemixer_fx_chan_t *>::iterator fi;

    for(fi=fx_channels_.begin(); fi!=fx_channels_.end(); fi++)
    {
        fi->second->set_curves(vol,pan);
    }

    master_controls_->set_curves(vol,pan);
}

piw::cookie_t piw::consolemixer_t::impl_t::create_fx_channel(unsigned num, const piw::cookie_t &output_cookie)
{
    std::map<unsigned, consolemixer_fx_chan_t *>::iterator i;

    if((i=fx_channels_.find(num))!=fx_channels_.end())
    {
        PIC_THROW("console mixer: effects send channel in use");
    }

#if CONSOLE_MIXER_DEBUG>0
    pic::logmsg() << "consolemixer_t::impl_t::create_fx_channel num="<<num;
#endif // CONSOLE_MIXER_DEBUG>0

    // TODO: create channels with different number of audio channels? stereo for now
    consolemixer_fx_chan_t *chan = new consolemixer_fx_chan_t(this, volfunc_, panfunc_, clk_domain_, output_cookie, num, 2);

    // make the channel upstream of the main mixdown
    // to ensure each channel mixdown happens first
    add_upstream(chan->input_clk_);

    // add to channel dictionary
    fx_channels_.insert(std::make_pair(num, chan));

    // add new fx chan to all other chans
    std::map<unsigned, consolemixer_chan_t *>::iterator fi;
    for(fi=channels_.begin(); fi!=channels_.end(); fi++)
        fi->second->add_fx_channel(num);

    // add new fx chan to all other fx chans, and all other fx chans to the new fx chan
    for(i=fx_channels_.begin(); i!=fx_channels_.end(); i++)
        if(i->first!=num)
        {
            i->second->add_fx_channel(num);
            fx_channels_[num]->add_fx_channel(i->first);
        }


//#define CONSOLE_MIXER_ENABLE_CHANS
#ifdef CONSOLE_MIXER_ENABLE_CHANS
    // TODO: -------------------------------------------------------------------------------------------------------
    // TODO: make all channels send enable
    // TODO: -------------------------------------------------------------------------------------------------------

    // make the all inputs upstream of the fx channel output clock
    std::map<unsigned, consolemixer_chan_t *>::iterator fi;
    for(fi=channels_.begin(); fi!=channels_.end(); fi++)
    {
        set_fx_send_enable(true, fi->second->num_, num, false);
    }

    // add all other fx chan input clocks upstream of all other fx chan output clocks
    for(i=fx_channels_.begin(); i!=fx_channels_.end(); i++)
        for(j=fx_channels_.begin(); j!=fx_channels_.end(); j++)
            if(i->first!=j->first)
            {
                set_fx_send_enable(true, i->second->num_, j->second->num_, true);
            }

    // TODO: -------------------------------------------------------------------------------------------------------
#endif // CONSOLE_MIXER_ENABLE_CHANS

    return cookie_t(chan);
}

void piw::consolemixer_t::impl_t::set_fx_send_enable(bool enable, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan)
{
    if(enable)
    {

        if(from_is_fx_chan)
        {
#if CONSOLE_MIXER_DEBUG>0
                pic::logmsg() << "consolemixer_t::impl_t::set_fx_send_enable attempting connection "
                              << from_chan << "->" << to_chan << "...";
#endif // CONSOLE_MIXER_DEBUG>0

            // ------- enable effects channel to effects channel send -------
            if(!fx_channels_[from_chan]->get_fx_send_enable(to_chan))
            {

                // ------- check to see if adding edge would create a cycle in the fx chan graph -------
#if CONSOLE_MIXER_DEBUG>0
                pic::logmsg() << "consolemixer_t::impl_t::set_fx_send_enable checking for cycle... ";
#endif // CONSOLE_MIXER_DEBUG>0

                std::map<unsigned, consolemixer_fx_chan_t *>::iterator i;
                // clear cycle flags in all chans
                for(i=fx_channels_.begin(); i!=fx_channels_.end(); i++)
                    i->second->walk_flag_ = false;

                // if there is already a path in the opposite direction of the added edge
                // then there will be a cycle
                bool cycle = fx_channels_[from_chan]->is_fx_node_upstream(fx_channels_[to_chan]);

                if(!cycle)
                {
                    // add edge fx channel graph
                    fx_channels_[to_chan]->add_upstream_fx_node(fx_channels_[from_chan]);
                    // make clock relationship to fx chan in clock graph
                    fx_channels_[from_chan]->add_fx_chan_downstream(fx_channels_[to_chan]);
                    // enable the send in the channel
                    fx_channels_[from_chan]->set_fx_send_enable(true, true, to_chan);
                }
                else
                {
#if CONSOLE_MIXER_DEBUG>0
                    // cycle, so can't have edge in chan dag or clock graph
                    // latency introduced between these channels!
                    pic::logmsg() << "consolemixer_t::impl_t::set_fx_send_enable fx chan clock cycle detected "
                                  << from_chan << "->" << to_chan;
#endif // CONSOLE_MIXER_DEBUG>0
                    // enable the send in the channel, without fx & clock graph connections
                    fx_channels_[from_chan]->set_fx_send_enable(true, false, to_chan);
                }

            }
#if CONSOLE_MIXER_DEBUG>0
            else
            {
                pic::logmsg() << "consolemixer_t::impl_t::set_fx_send_enable send already enabled "
                              << from_chan << "->" << to_chan;
            }
            pic::logmsg() << " ";
#endif // CONSOLE_MIXER_DEBUG>0

        }
        else
        {
            // ------- enable input channel to effects channel send -------

            if(!channels_[from_chan]->get_fx_send_enable(to_chan))
            {
                // make clock relationship, can always do this from an input channel
                channels_[from_chan]->add_fx_chan_downstream(fx_channels_[to_chan]);
                // enable the send in the channel

                channels_[from_chan]->set_fx_send_enable(true, true, to_chan);
            }

        }
    }
    else
    {

        if(from_is_fx_chan)
        {
            // ------- disable effects channel to effects channel send -------

            if(fx_channels_[from_chan]->get_fx_send_enable(to_chan))
            {
                // if a clock relationship was established
                if(fx_channels_[from_chan]->get_fx_send_clk_connection(to_chan))
                {
                    // remove edge fx channel graph
                    fx_channels_[to_chan]->remove_upstream_fx_node(fx_channels_[from_chan]);
                    // remove clock relationship to fx chan in clock graph
                    fx_channels_[from_chan]->remove_fx_chan_downstream(fx_channels_[to_chan]);
                }
                // disable the send in the channel
                fx_channels_[from_chan]->set_fx_send_enable(false, false, to_chan);
            }
        }
        else
        {
            // ------- disable input channel to effects channel send -------

            if(channels_[from_chan]->get_fx_send_enable(to_chan))
            {
                // remove clock relationship, can always do this from an input channel
                channels_[from_chan]->remove_fx_chan_downstream(fx_channels_[to_chan]);
                // disable the send in the channel
                channels_[from_chan]->set_fx_send_enable(false, false, to_chan);
            }

        }

    }

}

void piw::consolemixer_t::impl_t::set_fx_send_prefader(bool prefader, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan)
{
    channels_[from_chan]->set_fx_send_prefader(prefader, to_chan);
}


void piw::consolemixer_t::impl_t::remove_fx_channel(unsigned num)
{
    std::map<unsigned, consolemixer_chan_t *>::iterator i;
    std::map<unsigned, consolemixer_fx_chan_t *>::iterator fi;

    // remove fx chan from all other chans
    for(i=channels_.begin(); i!=channels_.end(); i++)
        i->second->remove_fx_channel(num);

    // remove fx chan from all other fx chans
    for(fi=fx_channels_.begin(); fi!=fx_channels_.end(); fi++)
        if(fi->first!=num)
            fi->second->remove_fx_channel(num);

    // remove from fx channels map
    if((fi=fx_channels_.find(num))!=fx_channels_.end())
    {
        // actually delete channel
        delete fi->second;
        // now remove from map
        fx_channels_.erase(fi);
        update_latency();
    }

}

void piw::consolemixer_t::impl_t::update_latency()
{
    unsigned l=0;
    std::map<unsigned, consolemixer_chan_t *>::iterator i=channels_.begin(), e=channels_.end();
    for(; i!=e; ++i)
    {
        l=std::max(l,i->second->get_latency());
    }
    std::map<unsigned, consolemixer_fx_chan_t *>::iterator fi=fx_channels_.begin(), fe=fx_channels_.end();
    for(; fi!=fe; ++fi)
    {
        l=std::max(l,fi->second->get_latency());
    }
    set_latency(l);
}

piw::cookie_t piw::consolemixer_t::impl_t::master_controls_cookie()
{
    return cookie_t(master_controls_);
}




/* --------------------------------------------------------------------------------------------------------------------------------------------------
 * consolemixer_t class methods
 *
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 */

piw::consolemixer_t::consolemixer_t(const pic::f2f_t &vol, const pic::f2f_t &pan, piw::clockdomain_ctl_t *d, const piw::cookie_t &c) : impl_(new impl_t(vol,pan,d,c))
{
}

piw::consolemixer_t::~consolemixer_t()
{
    delete impl_;
}

piw::cookie_t piw::consolemixer_t::create_channel(unsigned num)
{
    return impl_->create_channel(num);
}

void piw::consolemixer_t::remove_channel(unsigned num)
{
    impl_->remove_channel(num);
}

piw::cookie_t piw::consolemixer_t::create_fx_channel(unsigned num, const piw::cookie_t &output_cookie)
{
    return impl_->create_fx_channel(num, output_cookie);
}

void piw::consolemixer_t::remove_fx_channel(unsigned num)
{
    impl_->remove_fx_channel(num);
}

void piw::consolemixer_t::set_fx_send_enable(bool enable, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan)
{
    impl_->set_fx_send_enable(enable, from_chan, to_chan, from_is_fx_chan);
}

void piw::consolemixer_t::set_fx_send_prefader(bool prefader, unsigned from_chan, unsigned to_chan, bool from_is_fx_chan)
{
    impl_->set_fx_send_prefader(prefader, from_chan, to_chan, from_is_fx_chan);
}

piw::cookie_t piw::consolemixer_t::master_controls_cookie()
{
    return impl_->master_controls_cookie();
}

void piw::consolemixer_t::set_curves(const pic::f2f_t &vol, const pic::f2f_t &pan)
{
    impl_->set_curves(vol,pan);
}
