
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

#include <piw/piw_cfilter.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_clock.h>

#include <picross/pic_log.h>
#include <picross/pic_flipflop.h>

#include <vector>
#include <map>
#include <cstdio>

#define STATE_STOPPED 0
#define STATE_LINGER 1
#define STATE_RUNNING 2

namespace
{
    struct filter_wire_t;

    struct filter_holder_t
    {
        filter_holder_t(piw::cfilterctl_t *f, const piw::data_t &path): ctl_(f)
        {
            function_=ctl_->cfilterctl_create(path);
            inputs_=ctl_->cfilterctl_inputs();
            outputs_=ctl_->cfilterctl_outputs();
            thru_=ctl_->cfilterctl_thru();
        }
        ~filter_holder_t() { invalidate(); }
        void invalidate() { if(function_) ctl_->cfilterctl_delete(function_); function_=0; }
        piw::cfilterfunc_t *operator->() { PIC_ASSERT(function_); return function_; }

        piw::cfilterctl_t *ctl_;
        piw::cfilterfunc_t *function_;
        unsigned long long inputs_;
        unsigned long long outputs_;
        unsigned long long thru_;
    };

    struct filter_wire_t: piw::wire_t, piw::wire_ctl_t, piw::event_data_source_real_t, piw::event_data_sink_t, piw::cfilterenv_t, pic::element_t<>, virtual public pic::lckobject_t
    {
        filter_wire_t(piw::cfilter_t::impl_t *p, const piw::event_data_source_t &es);
        virtual ~filter_wire_t();
        void wire_closed();
        void changed(void *);
        void invalidate();
        void cfilterenv_output(unsigned signal, const piw::data_nb_t &data);
        piw::clockdomain_ctl_t *cfilterenv_clock();
        piw::clocksink_t *cfilterenv_clocksink();
        unsigned cfilterenv_latency();
        bool cfilterenv_next(unsigned &signal, piw::data_nb_t &value, unsigned long long max);
        bool cfilterenv_nextsig(unsigned signal, piw::data_nb_t &value, unsigned long long max);
        bool cfilterenv_latest(unsigned signal, piw::data_nb_t &value, unsigned long long max);
        void cfilterenv_reset(unsigned signal, unsigned long long max);
        void ticked(unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs);
        piw::data_t cfilterenv_path() { return name_; }
        void cfilterenv_dump(bool f) {  input_.dump(f); }

        static void __ticked(unsigned long long f,unsigned long long t,void *ctx);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &init);
        bool event_end(unsigned long long et);
        void source_ended(unsigned seq) { event_ended(seq); }
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);


        piw::cfilter_t::impl_t *parent_;
        piw::data_t name_;
        filter_holder_t holder_;
        unsigned state_;
        piw::dataholder_nb_t id_;
        piw::xevent_data_buffer_t input_;
        piw::xevent_data_buffer_t::iter_t iterator_;
        piw::xevent_data_buffer_t output_;
        bool downstopped_;
        unsigned seq_;
        unsigned long long event_end_time_;
        bool end_signalled_;
    };
}

struct piw::cfilter_t::impl_t: piw::root_t, piw::root_ctl_t, piw::clocksink_t, virtual public pic::lckobject_t
{
    impl_t(piw::cfilterctl_t *f, const piw::cookie_t &b, piw::clockdomain_ctl_t *d, bool linger_restart);
    ~impl_t();

    void root_closed();
    void root_opened();
    void root_clock();
    void root_latency();
    piw::wire_t *root_wire(const piw::event_data_source_t &es);

    void clocksink_ticked(unsigned long long f, unsigned long long t);
    void dochanged(void *);
    void changed(void *);
    void invalidate();

    piw::cfilterctl_t *ctl_;
    pic::flipflop_t<std::map<piw::data_t,filter_wire_t *> > children_;
    void *arg_;
    piw::clockdomain_ctl_t *domain_;
    bct_clocksink_t *up_;
    pic::ilist_t<filter_wire_t> clockers_;
    unsigned tick_count_;
    bool linger_restart_;
};

void filter_wire_t::event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    input_.set_signal(sig,n);
    iterator_->reset(sig,t);
}

void filter_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    end_signalled_ = false;
    id_.set_nb(id);
    input_ = b;
    iterator_ = b.iterator();
    output_.merge(b,holder_.thru_);

    bool c=holder_->cfilterfunc_start(this,id);

    seq_=seq;

    switch(state_)
    {
        case STATE_STOPPED:
        {
            if(c)
            {
                //pic::logmsg() << "cfilter running (was stopped)" << id.time() << " " << c;
                parent_->clockers_.append(this);
                parent_->tick_suppress(false);
                source_start(seq,id_,output_);
                state_=STATE_RUNNING;
            }

            break;
        }

        case STATE_RUNNING:
        {
            if(c)
            {
                //pic::logmsg() << "cfilter reinit (was running) " << id.time();
                source_start(seq,id,output_);
            }
            else
            {
                //pic::logmsg() << "cfilter stopped (was running) " << id.time();
                state_=STATE_STOPPED;
                remove();
                downstopped_=source_end(id.time());
            }

            break;
        }

        case STATE_LINGER:
        {
            if(c)
            {
                state_=STATE_RUNNING;
                //pic::logmsg() << "cfilter reinit (was lingering) " << id.time();
                if(parent_->linger_restart_)
                     source_start(seq,id,output_);
            }
            else
            {
                //pic::logmsg() << "cfilter stopped (was lingering) " << id.time();
                state_=STATE_STOPPED;
                remove();
                downstopped_=source_end(id.time());
            }

            break;
        }
    }
}

bool filter_wire_t::event_end(unsigned long long t)
{
    //pic::logmsg() << "filter " << (void *)this << " event_end at " << t;
    if(state_==STATE_RUNNING)
    {
        bool c=holder_->cfilterfunc_end(this,t);

        if(c)
        {
            state_=STATE_LINGER;
            event_end_time_ = t;
            //pic::logmsg() << "cfilter lingering (was running) " << t;
            return false;
        }
        else
        {
            //pic::logmsg() << "cfilter stopped (was running)" << t;
            state_=STATE_STOPPED;
            parent_->tick2(__ticked,this);
            remove();
            return downstopped_=source_end(t);
        }
    }

    if(state_==STATE_LINGER)
    {
        return false;
    }

    return downstopped_;
}

piw::cfilter_t::impl_t::impl_t(cfilterctl_t *f, const cookie_t &b, piw::clockdomain_ctl_t *d, bool linger_restart): root_t(0), ctl_(f), domain_(d), up_(0), tick_count_(0), linger_restart_(linger_restart)
{
    connect(b);
    domain_->sink(this,"filter");
    set_clock(this);
    tick_enable(true);
}

static int __changer(void *f_, void *_)
{
    piw::cfilter_t::impl_t *f = (piw::cfilter_t::impl_t *)f_;
    f->changed(f->arg_);
    return 0;
}

void piw::cfilter_t::impl_t::dochanged(void *arg)
{
    arg_=arg;
    tsd_fastcall(__changer,this,0);
}

void piw::cfilter_t::impl_t::root_clock()
{
    bct_clocksink_t *c = get_clock();
    if(c==up_)
    {
        return;
    }
    if(up_)
    {
        remove_upstream(up_);
    }
    up_ = c;
    if(up_)
    {
        add_upstream(up_);
    }
}

void piw::cfilter_t::impl_t::root_opened()
{
    root_clock();
    root_latency();
}

void piw::cfilter_t::impl_t::root_closed()
{
    invalidate();
    root_clock();
    root_latency();
}

void piw::cfilter_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    filter_wire_t *w = clockers_.head();
    unsigned count=0;
    unsigned long sr = get_sample_rate();
    unsigned bs = get_buffer_size();

    while(w)
    {
        count++;
        filter_wire_t *n = clockers_.next(w);
        w->ticked(f,t,sr,bs);
        w = n;
    }

    if(++tick_count_==6000)
    {
        tick_count_=0;
        pic::logmsg() << (void *)this << ": " << count << " active wires";
    }

    if(!clockers_.head())
    {
        tick_suppress(true);
        //pic::logmsg() << (void *)this << ": clock suppressed";
    }
}

void piw::cfilter_t::impl_t::changed(void *a)
{
    pic::flipflop_t<std::map<piw::data_t,filter_wire_t *> >::guard_t g(children_);
    std::map<piw::data_t,filter_wire_t *>::const_iterator ci = g.value().begin();

    for(;ci!=g.value().end();++ci)
    {
        ci->second->changed(a);
    }
}

void piw::cfilter_t::impl_t::invalidate()
{
    std::map<piw::data_t,filter_wire_t *>::iterator ci;

    while((ci=children_.alternate().begin())!=children_.alternate().end())
    {
        delete ci->second;
    }
}

piw::wire_t *piw::cfilter_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,filter_wire_t *>::iterator ci;

    if((ci=children_.alternate().find(es.path()))!=children_.alternate().end())
    {
        delete ci->second;
    }

    return new filter_wire_t(this,es);
}

piw::cfilter_t::impl_t::~impl_t()
{
    invalidate();
    tick_disable();
}

void piw::cfilter_t::impl_t::root_latency()
{
    set_sink_latency(get_latency());
    set_latency(get_latency()+ctl_->cfilterctl_latency());
}

filter_wire_t::filter_wire_t(piw::cfilter_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), parent_(p), name_(es.path()), holder_(parent_->ctl_,es.path()), state_(STATE_STOPPED)
{
    output_ = piw::xevent_data_buffer_t(holder_.outputs_|holder_.thru_,PIW_DATAQUEUE_SIZE_ISO);
    parent_->children_.alternate().insert(std::make_pair(name_,this));
    parent_->children_.exchange();
    parent_->connect_wire(this,source());
    subscribe(es);
}

void filter_wire_t::ticked(unsigned long long f, unsigned long long t,unsigned long sr, unsigned bs)
{
    if(!holder_->cfilterfunc_process(this,f,t,sr,bs))
    {
        //pic::logmsg() << "cfilter stopped due to inactivity " << f << "->" << t;
        state_=STATE_STOPPED;
        remove();
        downstopped_ = source_end(t);
        if(downstopped_)
        {
            event_ended(seq_);
        }
    }
}

filter_wire_t::~filter_wire_t()
{
    invalidate();
}

void filter_wire_t::wire_closed()
{
    delete this;
}

void filter_wire_t::changed(void *a)
{
    holder_->cfilterfunc_changed(this,a);
}

static int __invalidator(void *w_, void *_)
{
    filter_wire_t *w = (filter_wire_t *)w_;
    w->remove();
    w->state_=STATE_STOPPED;
    return 0;
}

void filter_wire_t::invalidate()
{
    if(parent_)
    {
        piw::tsd_fastcall(__invalidator,this,0);
        source_shutdown();
        unsubscribe();
        piw::wire_ctl_t::disconnect();
        piw::wire_t::disconnect();
        holder_.invalidate();
        parent_->children_.alternate().erase(name_);
        parent_->children_.exchange();
        parent_ = 0;
    }
}

void filter_wire_t::cfilterenv_output(unsigned signal, const piw::data_nb_t &data)
{
    output_.add_value(signal,data);
}

piw::clocksink_t *filter_wire_t::cfilterenv_clocksink()
{
    return parent_;
}

piw::clockdomain_ctl_t *filter_wire_t::cfilterenv_clock()
{
    return parent_->domain_;
}

unsigned filter_wire_t::cfilterenv_latency()
{
    return parent_->get_latency();
}

bool filter_wire_t::cfilterenv_next(unsigned &signal, piw::data_nb_t &value, unsigned long long max)
{
    if(iterator_->next(holder_.inputs_,signal,value,max))
    {
        return true;
    }

    if(state_==STATE_LINGER && !end_signalled_ && max>=event_end_time_)
    {
        end_signalled_=true;
        signal=0;
        value=piw::makenull_nb(event_end_time_);
        return true;
    }

    return false;
}

bool filter_wire_t::cfilterenv_nextsig(unsigned signal, piw::data_nb_t &value, unsigned long long max)
{
    return iterator_->nextsig(signal,value,max);
}

bool filter_wire_t::cfilterenv_latest(unsigned signal, piw::data_nb_t &value, unsigned long long max)
{
    return iterator_->latest(signal,value,max);
}

void filter_wire_t::cfilterenv_reset(unsigned signal, unsigned long long max)
{
    iterator_->reset(signal,max);
}

void filter_wire_t::__ticked(unsigned long long f, unsigned long long t, void *ctx)
{
    filter_wire_t *w=(filter_wire_t *)ctx;
    unsigned long sr = w->parent_->get_sample_rate();
    unsigned bs = w->parent_->get_buffer_size();
    w->holder_->cfilterfunc_process(w,f,t,sr,bs);
}

piw::cfilter_t::cfilter_t(piw::cfilterctl_t *ctl, const piw::cookie_t &o, piw::clockdomain_ctl_t *d, bool linger_restart): root_(new impl_t(ctl,o,d,linger_restart))
{
}

piw::cookie_t piw::cfilter_t::cookie()
{
    return piw::cookie_t(root_);
}

void piw::cfilter_t::changed(void *a)
{
    root_->dochanged(a);
}

piw::clocksink_t *piw::cfilter_t::sink()
{
    return root_;
}

piw::cfilter_t::~cfilter_t()
{
    root_->invalidate();
    delete root_;
}


