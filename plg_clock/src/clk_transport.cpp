
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

#include "clk_transport.h"

#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>
#include <piw/piw_clock.h>

#define SONGTIME_MAX        1000
#define SONGTIME_RATIO      1000000.0 /* units of clock 1e6 == seconds */

namespace
{
    struct transport_sig_t: piw::signal_ctl_t, piw::fastdata_t
    {
        transport_sig_t(): fastdata_t(PLG_FASTDATA_SENDER)
        {
            tsd_fastdata(this);
            send(piw::makefloat_bounded_nb(1,0,0,0,0ULL));
        }
    };
};

struct clocks::transport_t::impl_t: piw::root_ctl_t, piw::wire_ctl_t, piw::clockdomain_t, piw::clocksink_t, piw::fastdata_t
{
    impl_t(const piw::cookie_t &c): piw::fastdata_t(PLG_FASTDATA_SENDER), running_(~0ULL), current_(0), zero_(0.0)
    {
        tsd_fastdata(this);
        tsd_clockdomain(this);
        set_latency(0);

        send(piw::pathnull(0));
        set_source(piw::makestring("*",0));
        set_clock(this);

        connect(c);
        connect_wire(this, "", this);

        connect_signal(&run_sig_, 1, &run_sig_);
        connect_signal(&song_sig_, 2, &song_sig_);

        sink(this,"transport");
        tick_enable(false);
    }

    ~impl_t()
    {
        close_sink();
    }

    static int rewind(void *m_, void *a) { impl_t *m = (impl_t *)m_; m->rewind_fast(); return 0; }
    static int start(void *m_, void *a) { impl_t *m = (impl_t *)m_; m->start_fast(); return 0; }
    static int stop(void *m_, void *a) { impl_t *m = (impl_t *)m_; m->stop_fast(); return 0; }

    float interpolate(unsigned long long now)
    {
        return zero_+((float)(now-offset_))/SONGTIME_RATIO;
    }

    bool running()
    {
        return current_ >= running_;
    }

    void rewind_fast()
    {
        bool r=running();
        stop_fast();
        zero_ = 0.0;
        if(r) start_fast();
    }

    void start_fast()
    {
        offset_ = current_;
        song_sig_.send(piw::makefloat_bounded_nb(SONGTIME_MAX, 0, 0, zero_, current_+2));
        run_sig_.send(piw::makefloat_bounded_nb(1,0,0,1,current_+3));
        running_ = current_+4;
    }

    void stop_fast()
    {
        run_sig_.send(piw::makefloat_bounded_nb(1,0,0,0,current_+1));
        running_ = ~0ULL;
        zero_ = interpolate(current_);
        pic::msg() << "transport stop in fast" << pic::log;
    }

    void clocksink_ticked(unsigned long long from,unsigned long long now)
    {
        if(running())
        {
            //printf("transport sending %f\n",interpolate(now));
            song_sig_.send(piw::makefloat_bounded_nb(SONGTIME_MAX, 0, 0, interpolate(now), now));
        }

        current_ = now;
    }

    unsigned long long running_;
    unsigned long long current_;
    unsigned long long offset_;
    float zero_;

    transport_sig_t song_sig_;
    transport_sig_t run_sig_;
};

clocks::transport_t::transport_t(const piw::cookie_t &c) : impl_(new impl_t(c)) {}
clocks::transport_t::~transport_t() { delete impl_; }
void clocks::transport_t::start() { piw::tsd_fastcall(impl_t::start,impl_,0); }
void clocks::transport_t::stop() { piw::tsd_fastcall(impl_t::stop,impl_,0); }
void clocks::transport_t::rewind() { piw::tsd_fastcall(impl_t::rewind,impl_,0); }
