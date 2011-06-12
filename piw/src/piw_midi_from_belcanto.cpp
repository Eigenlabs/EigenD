
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

/*
 * piw_midi_from_belcanto.cpp.cpp
 *
 */

#include <piw/piw_midi_from_belcanto.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_logtable.h>
#include <picross/pic_ilist.h>
#include <piw/piw_bundle.h>
#include <piw/piw_clockclient.h>
#include <piw/piw_midi_gm.h>

#include <cmath>
//#include <sys/param.h>

#include <iostream>
#include <iomanip>
using namespace std;

#define CLKSTATE_IDLE 0
#define CLKSTATE_SHUTDOWN 1
#define CLKSTATE_RUNNING 2
#define CLKSTATE_STARTUP 3

#define NOTE_CONST 17.31234f
#define BASE_NOTE 69.0f
#define BASE_FREQ 440.0f

#define IN_FREQUENCY 2
#define IN_VELOCITY 4
// TODO: check these
#define IN_MASK SIG2(IN_FREQUENCY,IN_VELOCITY)

#define MIDI_FROM_BELCANTO_DEBUG 0

#define DEFAULT_CTRL_INTERVAL 10000ULL

// TODO:
// 1. test unconnecting and event ending downstream

#define CHANNEL_MIN 1
#define CHANNEL_MAX 16

namespace
{
    struct belcanto_note_wire_t;

    static unsigned char extract_keynum(const piw::data_nb_t &id)
    {
        unsigned char keynum = 0;
        unsigned lp = id.as_pathgristlen();
        if(lp>0)
        {
            const unsigned char *p = id.as_pathgrist();
            keynum = p[lp-1];
        }
        return keynum;
    }

    struct channel_list_t
    {
        channel_list_t(): min_(CHANNEL_MIN), max_(CHANNEL_MAX), legato_mode_(false), legato_initial_(false)
        {
            refresh_available_channels();
        }

        void refresh_available_channels()
        {
            list_.clear();
            for(unsigned i=min_; i<=max_; i++)
            {
                list_.push_back(i);
            }
        }

        void set_legato_mode(bool mode)
        {
            if(legato_mode_ != mode)
            {
                legato_mode_ = mode;
                legato_initial_ = true;
            }
        }

        unsigned get()
        {
            if(legato_mode_ && !legato_initial_)
            {
                return list_.back();
            }

            if(legato_mode_ && legato_initial_)
            {
                legato_initial_ = false;
            }

            pic::lcklist_t<unsigned>::nbtype::iterator i = list_.begin();
            if(i==list_.end()) return 1;

            list_.erase(i);
            list_.push_back(*i);

            return *i;
        }

        unsigned num_channels()
        {
            return (max_-min_)+1;
        }

        void put(unsigned c)
        {
            //pic::logmsg() << "midi put " << c;
            if(!legato_mode_)
            {
                list_.remove(c);
                list_.push_front(c);
            }
        }

        void set_min(unsigned c)
        {
            if(c<CHANNEL_MIN)
            {
                c = CHANNEL_MIN;
            }
            if(c<max_)
            {
                //pic::logmsg() << "midi min set to " << c;
                min_ = c;
            }
            refresh_available_channels();
        }

        void set_max(unsigned c)
        {
            if(c>CHANNEL_MAX)
            {
                c = CHANNEL_MAX;
            }
            if(c>min_)
            {
                //pic::logmsg() << "midi max set to " << c;
                max_ = c;
            }
            refresh_available_channels();
        }

        int get_channel(const piw::data_nb_t &id)
        {
            pic::lckmap_t<piw::data_nb_t,unsigned>::lcktype::iterator it;

            const unsigned char *p = id.as_path();
            int l = id.as_pathlen();

            for(;l>=0;l--)
            {
                piw::data_nb_t d = piw::makepath_nb(p,l);

                for(it=assignments_.begin() ; it!=assignments_.end(); it++)
                {
                   if(0==it->first.compare_path(d) && extract_keynum(d)==extract_keynum(it->first))
                   {
                     return (int)it->second;
                   }
                }
            }
            
            return 0;
        }

        void register_id(const piw::data_nb_t &id, unsigned channel)
        {
            if(channel<CHANNEL_MIN || channel>CHANNEL_MAX) return;
            assignments_.insert(std::make_pair(id,channel));
        }

        void unregister_id(const piw::data_nb_t &id)
        {
            assignments_.erase(id);
        }

        pic::lcklist_t<unsigned>::nbtype list_;
        unsigned min_;
        unsigned max_;
        bool legato_mode_;
        bool legato_initial_;
        pic::lckmap_t<piw::data_nb_t,unsigned>::lcktype assignments_;
    };

} // namespace

namespace piw
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi from belcanto implementation class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct midi_from_belcanto_t::impl_t: piw::root_t, piw::wire_ctl_t, piw::root_ctl_t, piw::event_data_source_real_t, piw::clocksink_t
    {
        impl_t(const piw::cookie_t &output_cookie, piw::clockdomain_ctl_t *clk_domain);
        ~impl_t() { invalidate(); }

        static int midi_from_belcanto_t::impl_t::init__(void *self_, void *arg_);

        void invalidate();

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

        void set_control_interval(float);

        // clocksink inherited functions:
        // ticked, convert the queued belcanto notes to MIDI notes
        void clocksink_ticked(unsigned long long from, unsigned long long to);

        void add_midi_data(bool global, unsigned status, unsigned d1, unsigned d2, unsigned long long t);
        void add_midi_data(bool global, unsigned status, unsigned d1, unsigned long long t);
        void set_resend_current(piw::resend_current_t);
        void dummy(const piw::data_nb_t &d) { /* dummy method for none existing notification */ }

        // queue up midi data from the input wires
        void add_to_midi_buffer(const piw::data_nb_t &d);
        // get the current channel, cycles through 16 possible channels
        unsigned get_channel();
        // set the MIDI channel to use
        void set_midi_channel(unsigned c);
        // set channel fast call, channel 0 sets poly mode (channel cycling)
        static int __setchannel(void *r_, void *c_);
        // set the MIDI program
        void set_program_change(unsigned c);
        // set the MIDI program fast call
        static int __set_program_change(void *r_, void *c_);
        // set the MIDI bank
        void set_bank_change(unsigned c);
        // set the MIDI bank fast call
        static int __set_bank_change(void *r_, void *c_);
        // set MIDI omni mode (send to all channels)
        void set_omni(bool b);
        // set MIDI omni mode fast call
        static int __set_omni(void *r_, void *b_);

        // check whether a status message has to be discarted
        bool check_time_status(unsigned channel, unsigned status, unsigned long long t);

        // set midi values
        void set_midi(pic::lckvector_t<midi_data_t>::nbtype &);
        void set_cc(bool global, bool continuous, unsigned channel, unsigned mid, unsigned lid, const unsigned d, unsigned long long t);

        // set other status MIDI messages
        void set_poly_aftertouch(bool global, bool continuous, unsigned channel, const unsigned noteid, const unsigned value, unsigned long long t);
        void set_program_change(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t);
        void set_channel_aftertouch(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t);
        void set_pitchbend(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t);
        void set_legato_trigger(const unsigned value, unsigned long long t);

        // input members:
        // input wires, one for each polyphonic input
        pic::lckmap_t<piw::data_t,belcanto_note_wire_t *,piw::path_less>::lcktype input_wires_;
        // active wires, wires that events have started on
        pic::ilist_t<belcanto_note_wire_t> active_input_wires_;
        // the upstream (root_ctl_t) clock sink
        bct_clocksink_t *upstream_clk_;

        // output members:
        // the downstream (root_t) clock sink
        bct_clocksink_t *downstream_clk_;
        // output MIDI data buffer
        piw::xevent_data_buffer_t output_buffer_;
         // signal mask
        unsigned long long sigmask_;

        // the clock sink state
        int clk_state_;
        // the clock domain
        piw::clockdomain_ctl_t *clk_domain_;

        // MIDI channel
        unsigned channel_;
        // channel list
        channel_list_t channel_list_;
        // poly mode - true: channel cycling, false: fixed channel
        bool poly_;
        // omni mode - true: send to all channels, false: poly mode
        bool omni_;
        // monotonically increasing packet timestamp
        unsigned long long time_;

        piw::resend_current_t resend_current_;

        unsigned long long ctrl_interval_;

        unsigned long long last_time_status_[MIDI_STATUS_MAX*CHANNEL_MAX];
        unsigned last_status_[MIDI_STATUS_MAX*CHANNEL_MAX];

        unsigned last_cc_msb_[MIDI_CC_MAX*CHANNEL_MAX];
        unsigned last_cc_lsb_[MIDI_CC_MAX*CHANNEL_MAX];

        bool send_notes_;
        bool send_pitchbend_;
    };

} // namespace piw


namespace
{

    struct belcanto_note_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<>
    {
        belcanto_note_wire_t(piw::midi_from_belcanto_t::impl_t *root, const piw::event_data_source_t &es);
        ~belcanto_note_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
        {
            iterator_->set_signal(sig,n);
            iterator_->reset(sig,t);
        }

        // decode signals to MIDI data and write to the MIDI data queue when ticked
        void ticked(unsigned long long from, unsigned long long to);
        // create MIDI data from various signal types...
        void set_velocity(const piw::data_nb_t &d, unsigned long long t);
        void set_frequency(const piw::data_nb_t &d, unsigned long long t, bool can_pitchbend);

        piw::midi_from_belcanto_t::impl_t *root_;
        piw::xevent_data_buffer_t::iter_t iterator_;
        unsigned channel_;
        piw::data_t path_;

        piw::data_nb_t id_;

        float note_id_;
        float note_velocity_;
        float note_pitch_;

        unsigned long long last_from_;
    };

    belcanto_note_wire_t::belcanto_note_wire_t(piw::midi_from_belcanto_t::impl_t *root, const piw::event_data_source_t &es):
        root_(root), path_(es.path()), note_id_(0)
    {
        root_->input_wires_.insert(std::make_pair(path_,this));
        subscribe(es);
    }

    void belcanto_note_wire_t::invalidate()
    {
        // unsubscribe from event data source
        unsubscribe();
        // disconnect this wire
        wire_t::disconnect();
        root_->input_wires_.erase(path_);
    }

    void belcanto_note_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "belcanto_note_wire_t::event_start";
#endif // MIDI_FROM_BELCANTO_DEBUG>0

        iterator_ = b.iterator();
        note_velocity_ = -1.f;
        note_pitch_ = -1.f;
        note_id_ = 0;

        channel_ = root_->get_channel();

        id_ = id;
        root_->channel_list_.register_id(id_, channel_);

        unsigned long long t = id.time();
        last_from_ = t;
        iterator_->reset(IN_VELOCITY,t);

        piw::data_nb_t d;
        if(iterator_->latest(IN_FREQUENCY,d,t)) set_frequency(d,t,true);

        root_->active_input_wires_.append(this);

        // ensure root clock is started up to handle new event
        if(root_->clk_state_ == CLKSTATE_IDLE)
        {
            root_->tick_suppress(false);
            root_->clk_state_ = CLKSTATE_STARTUP;
        }

        // notify that the current data can be resent in case the channels were in poly midi mode
        if(!root_->channel_)
        {
            root_->resend_current_(id);
        }
    }

    void belcanto_note_wire_t::ticked(unsigned long long from, unsigned long long to)
    {
#if MIDI_FROM_BELCANTO_DEBUG>1
        pic::logmsg() << "belcanto_note_wire_t::ticked";
#endif // MIDI_FROM_BELCANTO_DEBUG>1

        last_from_ = to;

        if(!iterator_.isvalid())
            return;

        bool can_pitchbend = root_->poly_ || root_->active_input_wires_.tail()==this;

        unsigned s;
        piw::data_nb_t d;
        while(iterator_->next(IN_MASK,s,d,to))
        {
            root_->time_ = std::max(root_->time_+1,d.time());

            if(s==IN_FREQUENCY)
            {
                set_frequency(d,root_->time_, can_pitchbend);
                continue;
            }

            if(s==IN_VELOCITY)
            {
                set_velocity(d,root_->time_);
                continue;
            }
        }

    }

    void belcanto_note_wire_t::set_velocity(const piw::data_nb_t &d, unsigned long long t)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "set_velocity: note_id_=" << note_id_ << " note_vel_=" << note_velocity_ << " note_pitch_=" << note_pitch_;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

        // return if a note already started
        if(note_id_)
        {
            return;
        }

        // the range is 1 to 127 to ensure a note on velocity 0 (=note off) is not sent
        note_velocity_ = 126.f*fabsf(d.as_norm()) + 1;

        if(note_pitch_>0)
        {
            note_id_ = note_pitch_;

            // create and queue midi data
            if(root_->send_notes_)
            {
                root_->add_midi_data(false, 0x90+channel_-1,(unsigned)note_id_,(unsigned)note_velocity_,t);
            }
        }
    }

    void belcanto_note_wire_t::set_frequency(const piw::data_nb_t &d, unsigned long long t, bool can_pitchbend)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "set_frequency: note_id_=" << note_id_ << " note_vel_=" << note_velocity_ << " note_pitch_=" << note_pitch_;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

        float f = d.as_renorm_float(BCTUNIT_HZ,0,96000,0);
        float n = (NOTE_CONST*std::log(f/BASE_FREQ))+BASE_NOTE;

        if(n>127.f)
            return;

        if(!note_id_)
        {
            // no note started
            note_pitch_ = (floorf(n + 0.5f));//roundf(n);

            if(note_velocity_>0)
            {
                note_id_ = note_pitch_;

                if(root_->send_notes_)
                {
                    root_->add_midi_data(false, 0x90+channel_-1,(unsigned)note_id_,(unsigned)note_velocity_,t++);
                }
            }
        }

        if(!note_id_)
            return;

        // pitch bending - a continuous stream of these MIDI packets is sent
        if(can_pitchbend && std::fabs(n-note_pitch_) > 1e-5)
        {
            float pb = (1.f+(n-note_pitch_))/2.f;
            pb = std::max(0.f,pb);
            pb = std::min(1.f,pb);

            if(root_->send_pitchbend_)
            {
                root_->set_pitchbend(false, true, channel_, pb*16383.f, t);
            }
        }
    }

    bool belcanto_note_wire_t::event_end(unsigned long long t)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "belcanto_note_wire_t::event_end";
#endif // MIDI_FROM_BELCANTO_DEBUG>0

        ticked(last_from_,t);
        iterator_.clear();
        if(note_id_)
        {
            root_->time_ = std::max(root_->time_+1,t);
            if(root_->send_notes_)
            {
                root_->add_midi_data(false, 0x80+channel_-1,(unsigned)note_id_,0x40,root_->time_);
            }
        }

        root_->channel_list_.unregister_id(id_);
        if(root_->poly_)
        {
            root_->channel_list_.put(channel_);
        }

        // remove from ilist
        remove();
        return true;
    }




} // namespace



namespace piw
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi from belcanto implementation functions
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_from_belcanto_t::impl_t::impl_t(const piw::cookie_t &cookie, piw::clockdomain_ctl_t *clk_domain):
        root_t(0), piw::event_data_source_real_t(piw::pathnull(0)),
        upstream_clk_(0), downstream_clk_(0), output_buffer_(1,4*PIW_DATAQUEUE_SIZE_NORM),
        clk_state_(CLKSTATE_IDLE), clk_domain_(clk_domain),
        channel_(1), poly_(false), omni_(false), time_(0ULL),
        resend_current_(piw::resend_current_t::method(this,&midi_from_belcanto_t::impl_t::dummy)),
        ctrl_interval_(DEFAULT_CTRL_INTERVAL), send_notes_(true), send_pitchbend_(true)
    {
        // one MIDI output channel
        sigmask_=1ULL;
        // set up the clocksink in the clock domain
        clk_domain->sink(this, "midi from belcanto");

        // pass this clock clocksink_t through the root_ctl_t to the downstream root_t
        // causes root_clock() to be called to establish the clock relationship
        set_clock(this);

        // connect the output cookie, have to do this before connecting the wire!
        connect(cookie);

        // connect this root_ctl_t to this event_data_source_real_t
        connect_wire(this, source());

        // enable the clock but suppress ticking
        this->tick_enable(true);

        piw::tsd_fastcall(init__,this,0);

        for(unsigned i=0;i<MIDI_STATUS_MAX*CHANNEL_MAX;i++)
        {
            last_time_status_[i]=0ULL;
            last_status_[i]=UINT_MAX;
        }

        for(unsigned i=0;i<MIDI_CC_MAX*CHANNEL_MAX;i++)
        {
            last_cc_msb_[i]=128;
            last_cc_lsb_[i]=128;
        }
    }

    int midi_from_belcanto_t::impl_t::init__(void *self_, void *arg_)
    {
        midi_from_belcanto_t::impl_t *self = (midi_from_belcanto_t::impl_t *)self_;

        // start the source, it will be held open for the lifetime of the agent
        // as we do not know when midi data will be stopping
        // (i.e. source runs even for note off data from a wire event end)
        self->source_start(0, piw::pathnull_nb(piw::tsd_time()), self->output_buffer_);
        
        return 1;
    }

    void midi_from_belcanto_t::impl_t::invalidate()
    {
        // shutdown the output event_data_source_t
        source_shutdown();

        // disconnect the root from upstream root_ctl_t
        piw::root_t::disconnect();

        // destruct all inputs - invalidates them also
        pic::lckmap_t<piw::data_t,belcanto_note_wire_t *,piw::path_less>::lcktype::iterator i;
        while((i=input_wires_.begin())!=input_wires_.end())
        {
            delete i->second;
        }

        // remove clock of upstream root_ctl_t
        if(upstream_clk_)
        {
            remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }
    }

    piw::wire_t *midi_from_belcanto_t::impl_t::root_wire(const piw::event_data_source_t &es)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "midi_from_belcanto::root_wire adding wire";
#endif // MIDI_FROM_BELCANTO_DEBUG>0
        pic::lckmap_t<piw::data_t,belcanto_note_wire_t *,piw::path_less>::lcktype::iterator i = input_wires_.find(es.path());
        if(i!=input_wires_.end())
        {
            delete i->second;
        }
        return new belcanto_note_wire_t(this,es);
    }

    void midi_from_belcanto_t::impl_t::root_clock()
    {
        // get the root_ctl_t clock of root_t
        bct_clocksink_t *s(get_clock());

        // if old upstream clock then remove
        if(upstream_clk_)
        {
            this->remove_upstream(upstream_clk_);
            upstream_clk_ = 0;
        }
        // save clock of upstream root_ctl_t
        if(s)
        {
            upstream_clk_ = s;
            this->add_upstream(s);
#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "midi_from_belcanto::root_clock add upstream " << s;
#endif // MIDI_FROM_BELCANTO_DEBUG>0
        }
    }

    void midi_from_belcanto_t::impl_t::root_opened()
    {
        root_clock();
        root_latency();
    }

    void midi_from_belcanto_t::impl_t::root_latency()
    {
        this->set_sink_latency(get_latency());
    }

    void midi_from_belcanto_t::impl_t::root_closed() {}

    void midi_from_belcanto_t::impl_t::add_to_midi_buffer(const piw::data_nb_t &d)
    {
#if MIDI_FROM_BELCANTO_DEBUG>1
        pic::logmsg() << "midi_from_belcanto_t::impl_t::add_to_midi_buffer " << d << ", " << d.time();
#endif // MIDI_FROM_BELCANTO_DEBUG>1

        output_buffer_.add_value(1, d);
    }

    unsigned midi_from_belcanto_t::impl_t::get_channel()
    {
        if(poly_)
        {
            return channel_list_.get();
        }
        return channel_;
    }

    void midi_from_belcanto_t::impl_t::set_resend_current(piw::resend_current_t resend_current)
    {
        resend_current_ = resend_current;
    }

    void midi_from_belcanto_t::impl_t::set_midi_channel(unsigned c)
    {
        piw::tsd_fastcall(__setchannel,this,&c);
    }

    int midi_from_belcanto_t::impl_t::__setchannel(void *r_, void *c_)
    {
        midi_from_belcanto_t::impl_t *r = (midi_from_belcanto_t::impl_t *)r_;
        unsigned c = *(unsigned *)c_;
        if(c==0)
        {
            r->channel_ = 0;
            r->poly_ = true;
        }
        else
        {
            r->channel_ = c;
            r->poly_ = false;
        }
        return 0;
    }

    void midi_from_belcanto_t::impl_t::set_program_change(unsigned c)
    {
        piw::tsd_fastcall(__set_program_change,this,&c);
    }

    int midi_from_belcanto_t::impl_t::__set_program_change(void *r_, void *c_)
    {
        midi_from_belcanto_t::impl_t *r = (midi_from_belcanto_t::impl_t *)r_;
        unsigned c = *(unsigned *)c_;

        r->set_program_change(r->poly_, false, r->poly_ ? 1 : r->channel_, (c - 1) << 7, r->time_);

        return 0;
    }

    void midi_from_belcanto_t::impl_t::set_bank_change(unsigned c)
    {
        piw::tsd_fastcall(__set_bank_change,this,&c);
    }

    int midi_from_belcanto_t::impl_t::__set_bank_change(void *r_, void *c_)
    {
        midi_from_belcanto_t::impl_t *r = (midi_from_belcanto_t::impl_t *)r_;
        unsigned c = *(unsigned *)c_;

        r->set_cc(r->poly_, false, r->poly_ ? 1 : r->channel_, 0, 0, (c - 1) << 7, r->time_);

        return 0;
    }

    void midi_from_belcanto_t::impl_t::set_control_interval(float interval)
    {
        if(interval<0)
        {
            ctrl_interval_ = ~0ULL;
        }
        else
        {
            interval *= 1000.0;
            ctrl_interval_ = (unsigned long long)interval;
        }
    }

    void midi_from_belcanto_t::impl_t::set_omni(bool b)
    {
        piw::tsd_fastcall(__set_omni,this,&b);
    }

    int midi_from_belcanto_t::impl_t::__set_omni(void *r_, void *b_)
    {
        midi_from_belcanto_t::impl_t *r = (midi_from_belcanto_t::impl_t *)r_;
        bool b = *(bool *)b_;
        r->omni_ = b;

        return 0;
    }

    bool midi_from_belcanto_t::impl_t::check_time_status(unsigned channel, unsigned status, unsigned long long t)
    {
        if(ctrl_interval_ == ~0ULL)
        {
            return false;
        }

        unsigned channel_offset = MIDI_STATUS_MAX*(channel-1);
        if(t>=last_time_status_[status+channel_offset]+ctrl_interval_)
        {
            last_time_status_[status+channel_offset]=t;
            return true;
        }

        return false;
    }

    void midi_from_belcanto_t::impl_t::set_midi(pic::lckvector_t<midi_data_t>::nbtype &data)
    {
        pic::lckvector_t<piw::midi_data_t>::nbtype::iterator i,e;
        i = data.begin();
        e = data.end();
        for(; i!=e; ++i)
        {
            time_ = std::max(time_+1,i->time_);
            bool global = false;
            unsigned channel = 0;
            unsigned char keynum = extract_keynum(i->id_);
            if(!poly_)
            {
                switch(i->scope_)
                {
                    case CHANNEL_SCOPE:
                        if(i->channel_)
                        {
                            global = false;
                            channel = i->channel_;
                            break;
                        }
                    default:
                        global = false;
                        belcanto_note_wire_t *w = active_input_wires_.head();
                        if(i->mcc_ == MIDI_CC_MAX + POLY_AFTERTOUCH ||
                           !keynum ||
                           (w && extract_keynum(w->id_) == keynum))
                        {
                            channel = channel_;
                        }
                        else
                        {
                            channel = 0;
                        }
                        break;
                }
            }
            else
            {
                switch(i->scope_)
                {
                    case CHANNEL_SCOPE:
                        if(i->channel_)
                        {
                            global = false;
                            channel = i->channel_;
                            break;
                        }
                    case GLOBAL_SCOPE:
                        global = true;
                        channel = 1;
                        break;
                    case PERNOTE_SCOPE:
                        global = false;
                        channel = channel_list_.get_channel(i->id_);
                        break;
                }
            }

#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "midi msg global: " << global << ", channel: " << channel << ", continuous: " << i->continuous_ << ", mcc: " << (unsigned)i->mcc_ << ", lcc: " << (unsigned)i->lcc_ << ", value: " << i->value_ << ", time: " << time_ << ", id:" << i->id_;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

            if(channel > 0)
            {
                if(i->mcc_ < MIDI_CC_MAX)
                {
                    set_cc(global, i->continuous_, channel, i->mcc_, i->lcc_, i->value_, time_);
                }
                else
                {
                    unsigned status = i->mcc_ - MIDI_CC_MAX;
                    if(status < MIDI_STATUS_MAX)
                    {
                        switch(status)
                        {
                            case POLY_AFTERTOUCH:
                            {
                                belcanto_note_wire_t *w = active_input_wires_.head();
                                while(w)
                                {
                                    if(global || w->channel_==channel)
                                    {
                                        if(extract_keynum(w->id_) == keynum)
                                        {
                                            if(w->note_id_!=0)
                                            {
                                                set_poly_aftertouch(global, i->continuous_, channel, w->note_id_, i->value_, time_);
                                            }
                                            break;
                                        }
                                    }
                                    w = active_input_wires_.next(w);
                                }
                                break;
                            }
                            case PROGRAM_CHANGE:
                            {
                                set_program_change(global, i->continuous_, channel, i->value_, time_);
                                break;
                            }
                            case CHANNEL_AFTERTOUCH:
                            {
                                set_channel_aftertouch(global, i->continuous_, channel, i->value_, time_);
                                break;
                            }
                            case PITCH_WHEEL:
                            {
                                set_pitchbend(global, i->continuous_, channel, i->value_, time_);
                                break;
                            }
                        }
                    }
                    else
                    {
                        unsigned midi_eigend = status - MIDI_STATUS_MAX;
                        switch(midi_eigend)
                        {
                            case LEGATO_TRIGGER:
                            {
                                set_legato_trigger(i->value_, time_);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    void midi_from_belcanto_t::impl_t::set_cc(bool global, bool continuous, unsigned channel, unsigned mid, unsigned lid, const unsigned value, unsigned long long t)
    {
        if(mid >= MIDI_CC_MAX || lid >= MIDI_CC_MAX)
        {
            return;
        }

        unsigned msb = (value&0x3fff)>>7;

        bool sent_lsb = false;

        unsigned channel_offset = MIDI_CC_MAX*(--channel);

        // send LSB first
        if(lid)
        {
            unsigned lsb = value&0x7f;
            if(!continuous || lsb!=last_cc_lsb_[lid+channel_offset])
            {
                // must increment the time to make sure both packets are sent
                add_midi_data(global, 0xb0+channel,lid,lsb,t);

                last_cc_lsb_[lid+channel_offset] = lsb;
                sent_lsb = true;
                time_ = std::max(time_+1,t++);
                t = time_;
            }
        }

        if(!continuous || sent_lsb || msb!=last_cc_msb_[mid+channel_offset])
        {
            // send MSB second so MIDI learn will pick MSB
            add_midi_data(global, 0xb0+channel,mid,msb,t);

            last_cc_msb_[mid+channel_offset] = msb;
        }
    }

    void midi_from_belcanto_t::impl_t::set_poly_aftertouch(bool global, bool continuous, unsigned channel, const unsigned noteid, const unsigned value, unsigned long long t)
    {
        unsigned channel_offset = MIDI_STATUS_MAX*(--channel);

        unsigned msb = (value&0x3fff)>>7;
        unsigned composite_value = ((noteid&0x7f)<<7) + msb; 
        if(!continuous || composite_value!=last_status_[POLY_AFTERTOUCH+channel_offset])
        {

            add_midi_data(global, 0xa0+channel, noteid, msb, t);
            last_status_[POLY_AFTERTOUCH+channel_offset] = composite_value;
        }
    }

    void midi_from_belcanto_t::impl_t::set_program_change(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t)
    {
        unsigned channel_offset = MIDI_STATUS_MAX*(--channel);

        unsigned msb = (value&0x3fff)>>7;

        if(!continuous || msb!=last_status_[PROGRAM_CHANGE+channel_offset])
        {
            add_midi_data(global, 0xc0+channel, msb, t);
            last_status_[PROGRAM_CHANGE+channel_offset] = msb;
        }
    }

    void midi_from_belcanto_t::impl_t::set_channel_aftertouch(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t)
    {
        unsigned channel_offset = MIDI_STATUS_MAX*(--channel);

        unsigned msb = (value&0x3fff)>>7;

        if(!continuous || msb!=last_status_[CHANNEL_AFTERTOUCH+channel_offset])
        {
            add_midi_data(global, 0xd0+channel, msb, t);
            last_status_[CHANNEL_AFTERTOUCH+channel_offset] = msb;
        }
    }

    void midi_from_belcanto_t::impl_t::set_pitchbend(bool global, bool continuous, unsigned channel, const unsigned value, unsigned long long t)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "set_pitchbend global=" << global << " continuous=" << continuous << " channel=" << channel << " value=" << value << " time=" << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0
        if(continuous && !check_time_status(channel,PITCH_WHEEL,t))
        {
            return;
        }

        unsigned channel_offset = MIDI_STATUS_MAX*(--channel);
        if(!continuous || value!=last_status_[PITCH_WHEEL+channel_offset])
        {
            unsigned msb = (value&0x3fff)>>7;
            unsigned lsb = value&0x7f;

            add_midi_data(global, 0xe0+channel, lsb, msb, t);
            last_status_[PITCH_WHEEL+channel_offset] = value;
        }
    }

    void midi_from_belcanto_t::impl_t::add_midi_data(bool global, unsigned status, unsigned d1, unsigned long long t)
    {
        if(!omni_ && !global)
        {
            // send to one channel
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(t,2,&blob);

            blob[0] = (unsigned char)status;
            blob[1] = (unsigned char)d1;
#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "add_midi_data d0=" << hex << (unsigned)status << " d1=" << (unsigned)d1 << " len=2 time=" << std::dec << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

            add_to_midi_buffer(d);
        }
        else
        {
            // omni mode, send to all channels
            for(unsigned channel = channel_list_.min_-1; channel < channel_list_.max_; channel++)
            {
                unsigned char *blob = 0;
                piw::data_nb_t d = piw::makeblob_nb(t,2,&blob);

                blob[0] = (unsigned char)((status&0xf0)+channel);
                blob[1] = (unsigned char)d1;
#if MIDI_FROM_BELCANTO_DEBUG>0
                pic::logmsg() << "add_midi_data d0=" << hex << (unsigned)blob[0] << " d1=" << (unsigned)blob[1] << " time=" << std::dec << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

                add_to_midi_buffer(d);

                time_ = std::max(time_+1,t++);
                t = time_;
            }
        }
    }

    void midi_from_belcanto_t::impl_t::set_legato_trigger(const unsigned value, unsigned long long t)
    {
#if MIDI_FROM_BELCANTO_DEBUG>0
        pic::logmsg() << "set_legato_trigger value=" << value << " time=" << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

        channel_list_.set_legato_mode(value >= 0x1fff);
    }

    void midi_from_belcanto_t::impl_t::add_midi_data(bool global, unsigned status, unsigned d1, unsigned d2, unsigned long long t)
    {
        if(!omni_ && !global)
        {
            // send to one channel
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(t,3,&blob);

            blob[0] = (unsigned char)status;
            blob[1] = (unsigned char)d1;
            blob[2] = (unsigned char)d2;
#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "add_midi_data d0=" << hex << (unsigned)status << " d1=" << (unsigned)d1 << " d2=" << (unsigned)d2 << " len=3 time=" << std::dec << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

            add_to_midi_buffer(d);
        }
        else
        {
            // omni mode, send to all channels
            for(unsigned channel = channel_list_.min_-1; channel < channel_list_.max_; channel++)
            {
                unsigned char *blob = 0;
                piw::data_nb_t d = piw::makeblob_nb(t,3,&blob);
            
                blob[0] = (unsigned char)((status&0xf0)+channel);
                blob[1] = (unsigned char)d1;
                blob[2] = (unsigned char)d2;
#if MIDI_FROM_BELCANTO_DEBUG>0
                pic::logmsg() << "add_midi_data d0=" << hex << (unsigned)blob[0] << " d1=" << (unsigned)d1 << " d2=" << (unsigned)d2 << " len=3 time=" << std::dec << t;
#endif // MIDI_FROM_BELCANTO_DEBUG>0

                add_to_midi_buffer(d);

                time_ = std::max(time_+1,t++);
                t = time_;
            }
        }
    }

    void midi_from_belcanto_t::impl_t::clocksink_ticked(unsigned long long from, unsigned long long to)
    {
#if MIDI_FROM_BELCANTO_DEBUG>1
            pic::logmsg() << "midi_from_belcanto_t::impl_t::clocksink_ticked";
#endif // MIDI_FROM_BELCANTO_DEBUG>1

        belcanto_note_wire_t *w = active_input_wires_.head();

        // if no active wires then shut down
        if(!w)
        {
            clk_state_=CLKSTATE_SHUTDOWN;
        }

        // tick all the active wires and queue up MIDI data
        while(w)
        {
            w->ticked(from, to);
            w = active_input_wires_.next(w);
        }

        // manage start up and shut down of clock
        // clock startup now
        if(clk_state_==CLKSTATE_STARTUP)
        {
            clk_state_=CLKSTATE_RUNNING;
#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "midi_from_belcanto_t::impl_t clk running";
#endif // MIDI_FROM_BELCANTO_DEBUG>0
        }


        if(clk_state_==CLKSTATE_SHUTDOWN)
        {
            // shutdown clock
            this->tick_suppress(true);
            clk_state_=CLKSTATE_IDLE;

#if MIDI_FROM_BELCANTO_DEBUG>0
            pic::logmsg() << "midi_from_belcanto_t::impl_ clk idle";
#endif // MIDI_FROM_BELCANTO_DEBUG>0
        }
    }

    static int __set_min_midi_channel(void *i_, void *c_)
    {
        midi_from_belcanto_t::impl_t *i = (midi_from_belcanto_t::impl_t *)i_;
        unsigned c = *(unsigned *)c_;
        i->channel_list_.set_min(c);
        return 0;
    }

    static int __set_max_midi_channel(void *i_, void *c_)
    {
        midi_from_belcanto_t::impl_t *i = (midi_from_belcanto_t::impl_t *)i_;
        unsigned c = *(unsigned *)c_;
        i->channel_list_.set_max(c);
        return 0;
    }

    static int __set_send_notes(void *i_, void *send_)
    {
        midi_from_belcanto_t::impl_t *i = (midi_from_belcanto_t::impl_t *)i_;
        bool send = *(bool *)send_;
        i->send_notes_ = send;
        return 0;
    }

    static int __set_send_pitchbend(void *i_, void *send_)
    {
        midi_from_belcanto_t::impl_t *i = (midi_from_belcanto_t::impl_t *)i_;
        bool send = *(bool *)send_;
        i->send_pitchbend_ = send;
        return 0;
    }

    static int __get_channel(void *i_, void *d_)
    {
        midi_from_belcanto_t::impl_t *i = (midi_from_belcanto_t::impl_t *)i_;
        const piw::data_nb_t d = *(const piw::data_nb_t *)i_;
        return i->channel_list_.get_channel(d);
    }


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi from belcanto interface class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_from_belcanto_t::midi_from_belcanto_t(const piw::cookie_t &cookie, piw::clockdomain_ctl_t *clk_domain) : impl_(new impl_t(cookie, clk_domain)) {}
    midi_from_belcanto_t::~midi_from_belcanto_t() { delete impl_; }

    piw::cookie_t midi_from_belcanto_t::cookie() { return piw::cookie_t(impl_); }

    void midi_from_belcanto_t::set_resend_current(piw::resend_current_t f) { impl_->set_resend_current(f); }
    void midi_from_belcanto_t::set_midi_channel(unsigned c) { impl_->set_midi_channel(c); }
    void midi_from_belcanto_t::set_min_midi_channel(unsigned c) { piw::tsd_fastcall(__set_min_midi_channel,impl_,&c); }
    void midi_from_belcanto_t::set_max_midi_channel(unsigned c) { piw::tsd_fastcall(__set_max_midi_channel,impl_,&c); }
    void midi_from_belcanto_t::set_omni(bool b) { impl_->set_omni(b); }
    void midi_from_belcanto_t::set_program_change(unsigned p) { impl_->set_program_change(p); }
    void midi_from_belcanto_t::set_bank_change(unsigned b) { impl_->set_bank_change(b); }
    void midi_from_belcanto_t::set_control_interval(float interval) { impl_->set_control_interval(interval); }
    void midi_from_belcanto_t::set_midi(pic::lckvector_t<midi_data_t>::nbtype &data) { impl_->set_midi(data); }
    void midi_from_belcanto_t::set_send_notes(bool send) { piw::tsd_fastcall(__set_send_notes,impl_,&send); }
    void midi_from_belcanto_t::set_send_pitchbend(bool send) { piw::tsd_fastcall(__set_send_pitchbend,impl_,&send); }
    unsigned midi_from_belcanto_t::get_active_midi_channel(const piw::data_nb_t &d) { return piw::tsd_fastcall(__get_channel,impl_,(void *)&d); }

} // namespace piw



