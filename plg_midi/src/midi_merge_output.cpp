
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
 * midi_mergeout.cpp: merge multiple MIDI output streams into a single stream of MIDI data
 *
 *
 */

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_bundle.h>
#include <piw/piw_cfilter.h>
#include <piw/piw_clock.h>

#include <picross/pic_time.h>
#include <picross/pic_log.h>

#include <picross/pic_cfrunloop.h>

#include <lib_midi/midi_decoder.h>

#include <plg_midi/midi_merge_output.h>

#include <iostream>
#include <iomanip>
using namespace std;


#define MIDI_MERGE_DEBUG 0

namespace
{
    struct midi_merge_output_ctl_t;
} // namespace

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// midi output implementation class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct pi_midi::midi_merge_output_t::impl_t: piw::clocksink_t
{
    impl_t(const piw::change_nb_t &sendmidi_ftr, piw::clockdomain_ctl_t *clk_domain);
    ~impl_t();

    // add incoming data from cfilter_funcs to the data queue
    void add_to_queue(const piw::data_nb_t &data);
    // increment the cfilter_func count, when >0 then impl clock runs
    void inc_func_count();
    // decrement the cfilter_func count
    void dec_func_count();
    // ticked after cfilter, send to MIDI port through a functor
    virtual void clocksink_ticked(unsigned long long f, unsigned long long t);

    pic::lckmultiset_t<piw::data_nb_t>::nbtype merge_queue_;

    piw::change_nb_t midi_output_functor_;
    midi_merge_output_ctl_t *midi_merge_out_ctl_;

    bool clk_running_;
    unsigned func_count_;

};


namespace
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi merge output cfilterfunc class
    //
    // an output midi stream
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct midi_merge_output_func_t: piw::cfilterfunc_t
    {
        midi_merge_output_func_t(pi_midi::midi_merge_output_t::impl_t *impl_);

        bool cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id);
        bool cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs);
        bool cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time);

        pi_midi::midi_merge_output_t::impl_t *impl_;

    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi merge output cfilterctl class
    //
    // merges down output MIDI streams to be sent to the MIDI output ports
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    struct midi_merge_output_ctl_t: piw::cfilterctl_t, piw::cfilter_t
    {

        midi_merge_output_ctl_t(piw::clockdomain_ctl_t *clk_domain, pi_midi::midi_merge_output_t::impl_t *impl)
            : cfilter_t(this, piw::cookie_t(0), clk_domain), impl_(impl) {}

        piw::cfilterfunc_t *cfilterctl_create(const piw::data_t &path)
        {
#if MIDI_MERGE_DEBUG>0
            unsigned num = path.as_path()[0];
            pic::logmsg() << "midimergeout_ctl_t::cfilterctl_create " << num;
#endif // MIDI_MERGE_DEBUG>0
            midi_merge_output_func_t *func = new midi_merge_output_func_t(impl_);

            // return a new func
            return func;
        }

        void cfilterctl_delete(piw::cfilterfunc_t *f)
        {
#if MIDI_MERGE_DEBUG>0
            pic::logmsg() << "midimergeout_ctl_t::cfilterctl_delete";
#endif // MIDI_MERGE_DEBUG>0
            cfilterctl_t::cfilterctl_delete(f);
        }

        pi_midi::midi_merge_output_t::impl_t *impl_;

    };


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi merge output cfilterfunc functions
    //
    // an output midi stream, adds data to queue in impl
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_merge_output_func_t::midi_merge_output_func_t(pi_midi::midi_merge_output_t::impl_t *impl) : impl_(impl)
    {
    }

    bool midi_merge_output_func_t::cfilterfunc_start(piw::cfilterenv_t *env, const piw::data_nb_t &id)
    {
#if MIDI_MERGE_DEBUG>0
        pic::logmsg() << "midi_merge_output_func_t::cfilterfunc_start";
#endif // MIDI_MERGE_DEBUG>0
        unsigned long long t=id.time();
        piw::data_nb_t d;

        if(env->cfilterenv_latest(1,d,t))
        {
            // get data from vector and add to queue for merging
            impl_->add_to_queue(d);
        }

        impl_->inc_func_count();

        return true;
    }

    bool midi_merge_output_func_t::cfilterfunc_process(piw::cfilterenv_t *env, unsigned long long from, unsigned long long to,unsigned long sr, unsigned bs)
    {
        piw::data_nb_t d;
        unsigned sig;

        while(env->cfilterenv_next(sig,d,to))
        {
            if(sig==1)
            {
//                unsigned char *d2 = (unsigned char *)d.as_blob();
//                pic::logmsg() << "midi_merge_output  d0=" << hex << (unsigned)d2[0] << " d1=" << (unsigned)d2[1] << " d2=" << (unsigned)d2[2] << " len=3";

                // get data from vector and add to queue for merging
                impl_->add_to_queue(d);
            }
        }

        return true;
    }

    bool midi_merge_output_func_t::cfilterfunc_end(piw::cfilterenv_t *env, unsigned long long time)
    {
#if MIDI_MERGE_DEBUG>0
        pic::logmsg() << "midi_merge_output_func_t::cfilterfunc_end";
#endif // MIDI_MERGE_DEBUG>0
        impl_->dec_func_count();

        return false;
    }



} // namespace



namespace pi_midi
{
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi output implementation functions
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_merge_output_t::impl_t::impl_t(const piw::change_nb_t &sendmidi_ftr, piw::clockdomain_ctl_t *clk_domain):
            midi_output_functor_(sendmidi_ftr), clk_running_(false), func_count_(0)
    {
        midi_merge_out_ctl_ = new midi_merge_output_ctl_t(clk_domain, this);

        // add to the clock domain
        clk_domain->sink(this, "midi_merge_output");

        // put this downstream of cfilter
        add_upstream(midi_merge_out_ctl_->sink());

        // enable ticking but suppress it
        tick_enable(true);

    }

    midi_merge_output_t::impl_t::~impl_t()
    {
        remove_upstream(midi_merge_out_ctl_->sink());

        delete midi_merge_out_ctl_;

        tick_disable();
    }

    void midi_merge_output_t::impl_t::add_to_queue(const piw::data_nb_t &data)
    {
        // TODO: are times needed here?
        merge_queue_.insert(data);
    }

    void midi_merge_output_t::impl_t::inc_func_count()
    {
        if(func_count_==0)
        {
            tick_suppress(false);
            clk_running_=true;
        }
        func_count_++;
    }

    void midi_merge_output_t::impl_t::dec_func_count()
    {
        func_count_--;
        if(func_count_==0)
           clk_running_=false;
    }

    void midi_merge_output_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        pic::lckmultiset_t<piw::data_nb_t>::nbtype::iterator i;

#if MIDI_MERGE_DEBUG>1
        pic::logmsg() << "midi_merge_output_t::impl_t::clocksink_ticked";
#endif // MIDI_MERGE_DEBUG>1

        if(!merge_queue_.empty())
        {
            // call functor with midi data on merge queue and empty queue
            for(i=merge_queue_.begin();i!=merge_queue_.end();i++)
            {
                midi_output_functor_(*i);
                //unsigned char *d = (unsigned char *)data.as_blob();
                //pic::logmsg() << "midi_merge_output  d0=" << hex << (unsigned)d[0] << " d1=" << (unsigned)d[1] << " d2=" << (unsigned)d[2] << " len=3";
            }
            merge_queue_.clear();
        }

        if(!clk_running_)
            tick_suppress(true);

    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi output interface class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_merge_output_t::midi_merge_output_t(const piw::change_nb_t &midi_output_functor, piw::clockdomain_ctl_t *clk_domain):
        impl_(new impl_t(midi_output_functor, clk_domain)) {}

    midi_merge_output_t::~midi_merge_output_t()
    {
        delete impl_;
    }

    piw::cookie_t midi_merge_output_t::cookie() { return impl_->midi_merge_out_ctl_->cookie(); }



} // namespace pi_midi




