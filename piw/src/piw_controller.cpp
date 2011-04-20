
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

#include <piw/piw_controller.h>

#include <piw/piw_ufilter.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>

#include <picross/pic_log.h>
#include <picross/pic_flipflop.h>

#include <vector>

namespace
{
    struct ctlfilter_t: piw::ufilterfunc_t
    {
        ctlfilter_t(piw::controller_t::impl_t *controller): controller_(controller), knum_(0) {}
        void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id);
        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d);
        void ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long);

        piw::controller_t::impl_t *controller_;
        unsigned knum_;
        piw::dataholder_nb_t last_;
    };

    struct ctlsignal_t: piw::wire_ctl_t, piw::event_data_source_real_t
    {
        ctlsignal_t(piw::xxcontrolled_t *c, unsigned id): piw::event_data_source_real_t(piw::pathone(id+1,0)), id_(id), client_(c), state_(0), savestate_(0)
        {
        }

        ~ctlsignal_t()
        {
            source_shutdown();
        }

        void save()
        {
            savestate_=state_;
            state_=0;

            if(savestate_ != state_)
            {
                source_end(piw::tsd_time());
            }
        }

        void restore()
        {
            set(savestate_);
        }

        void source_ended(unsigned seq)
        {
        }

        void set(unsigned v)
        {
            if(v==state_)
            {
                return;
            }

            if(v)
            {
                if(state_)
                {
                    buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,v,piw::tsd_time()));
                }
                else
                {
                    buffer_=piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
                    unsigned long long t = piw::tsd_time();
                    buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,v,t));
                    source_start(0,piw::pathone_nb(id_,t+1),buffer_);
                }
            }
            else
            {
                piw::tsd_fastcall(__unset,this,0);
            }

            state_=v;
        }

        static int __unset(void *self_, void *)
        {
            ctlsignal_t *self = (ctlsignal_t *)self_;

            if(self->state_)
            {
                self->source_end(piw::tsd_time());
            }

            return 1;
        }

        unsigned id_;
        piw::xxcontrolled_t *client_;
        unsigned state_,savestate_;
        piw::xevent_data_buffer_t buffer_;
    };

    static int __init(void *xctl_, void *)
    {
        piw::xxcontrolled_t *xctl = (piw::xxcontrolled_t *)xctl_;
        xctl->control_init();
        return 0;
    }

}

struct piw::xxcontrolled_t::impl_t
{
    impl_t() : queue_(tsd_dataqueue(PIW_DATAQUEUE_SIZE_NORM)), fastdata_(PLG_FASTDATA_SENDER)
    {
        tsd_fastdata(&fastdata_);
    }
    
    fastdata_t *fastdata() { return &fastdata_; }
    change_nb_t sender() { return queue_.sender_fast(); }
    void send(const piw::data_nb_t &d) { queue_.write_fast(d); }
    void start_event(unsigned long long t) { fastdata_.send_fast(pathnull_nb(t),queue_);  }
    void end_event(unsigned long long t)   { fastdata_.send_fast(makenull_nb(t),dataqueue_t()); }

    dataqueue_t queue_;
    fastdata_t fastdata_;
};

struct piw::controller_t::impl_t: piw::ufilterctl_t, piw::root_ctl_t
{
    impl_t(piw::controller_t *h,const piw::cookie_t &c,const std::string &sigmap): filter_(this,piw::cookie_t(0)),sigmask_(0), host_(h)
    {
        std::string::const_iterator i,e;
        i=sigmap.begin();
        e=sigmap.end();

        for(; i!=e; ++i)
        {
            unsigned si = (*i)-1;
            sigmask_ |= (1ULL<<(si));
        }

        connect(c);
    }

    ~impl_t()
    {
    }

    unsigned long long ufilterctl_thru() { return 0; }
    unsigned long long ufilterctl_outputs() { return 0; }
    unsigned long long ufilterctl_inputs() { return sigmask_; }

    void start(piw::ufilterenv_t *env, unsigned long long time, unsigned index)
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);

        if(index<g.value().size())
        {
            ctlsignal_t *c = g.value()[index];

            if(c && c->client_)
            {
                c->client_->control_start(time);
            }
        }
    }

    void filter(piw::ufilterenv_t *env, unsigned input_index, const piw::data_nb_t &value, unsigned index)
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);

        if(index<g.value().size())
        {
            ctlsignal_t *c = g.value()[index];

            if(c && c->client_)
            {
                c->client_->control_receive(input_index-1,value);
            }
        }
    }

    void term(piw::ufilterenv_t *env, unsigned long long t, unsigned index)
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);

        if(index<g.value().size())
        {
            ctlsignal_t *c = g.value()[index];

            if(c && c->client_)
                c->client_->control_term(t);
        }
    }

    piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &)
    {
        ctlfilter_t *c = new ctlfilter_t(this);
        return c;
    }

    void ufilterctl_latency_in(unsigned l)
    {
        host_->controller_latency(l);
    }

    void ufilterctl_clock(bct_clocksink_t *c)
    {
        host_->controller_clock(c);
    }

    void set_light(unsigned index, unsigned value)
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);

        if(index<g.value().size())
        {
            ctlsignal_t *c = g.value()[index];

            if(c)
            {
                c->set(value);
            }
        }
    }

    void save_lights()
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);
        for(unsigned i=0; i<g.value().size(); ++i)
        {
            ctlsignal_t *c = g.value()[i];
            if(c)
            {
                c->save();
            }
        }
    }
    
    void restore_lights()
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);
        for(unsigned i=0; i<g.value().size(); ++i)
        {
            ctlsignal_t *c = g.value()[i];
            if(c)
            {
                c->restore();
            }
        }
    }

    bool attach_to(xxcontrolled_t *c, unsigned n)
    {
        std::vector<ctlsignal_t *> &l(controllist_.alternate());
        unsigned s=l.size();

        if(s<n+1)
        {
            l.resize(n+1);
        }

        if(l[n] && l[n]->client_)
        {
            return false;
        }

        ctlsignal_t *cs = new ctlsignal_t(c,n+1);
        connect_wire(cs,cs->source());
        l[n]=cs;
        controllist_.exchange();

        return true;
    }

    unsigned attach(xxcontrolled_t *c)
    {
        unsigned i;
        std::vector<ctlsignal_t *> &l(controllist_.alternate());
        unsigned s=l.size();

        for(i=0;i<s;i++)
        {
            if(!l[i])
            {
                goto found;
            }
            if(!l[i]->client_)
            {
                delete l[i];
                l[i]=0;
                goto found;
            }
        }

        i=s;
        l.resize(s+1);

    found:

        ctlsignal_t *cs = new ctlsignal_t(c,i+1);
        connect_wire(cs,cs->source());
        l[i]=cs;
        controllist_.exchange();
        return i;
    }

    void detach(unsigned index)
    {
        ctlsignal_t *s = controllist_.alternate()[index];
        controllist_.alternate()[index]=0;
        controllist_.exchange();
        piw::tsd_fastcall(__delete_signal,s,0);
    }

    static int __delete_signal(void *s_, void *)
    {
        ctlsignal_t *s = (ctlsignal_t *)s_;
        delete s;
        return 1;
    }

    piw::ufilter_t filter_;
    pic::flipflop_t<std::vector<ctlsignal_t *> > controllist_;
    unsigned long sigmask_;
    piw::controller_t *host_;
};

void ctlfilter_t::ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long t)
{
    if(knum_>0)
    {
        controller_->term(env,t,knum_-1);
    }
}

void ctlfilter_t::ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
{
    if(id.is_path() && id.as_pathlen()>0)
    {
        knum_=id.as_path()[id.as_pathlen()-1];
        env->ufilterenv_start(id.time());
        last_.clear();
        controller_->start(env,id.time(),knum_-1);
    }
    else
    {
        knum_=0;
    }
}

void ctlfilter_t::ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d)
{
    if(knum_!=0)
    {
        if(d.compare(last_.get(),false)!=0)
        {
            last_.set_nb(d);
            controller_->filter(env,s,d,knum_-1);
        }
    }
}

piw::controller_t::controller_t(const piw::cookie_t &c,const std::string &sigmap): impl_(new impl_t(this,c,sigmap)) { } 
piw::controller_t::~controller_t() { delete impl_; } 

piw::xxcontrolled_t::xxcontrolled_t() : controller_(0), xximpl_(new impl_t()) { }
piw::xxcontrolled_t::~xxcontrolled_t()
{
    piw::tsd_fastcall(__destruct,this,0);
}

int piw::xxcontrolled_t::__destruct(void *self_, void *)
{
    xxcontrolled_t *self = (xxcontrolled_t *)self_;
    delete self->xximpl_;

    return 1;
} 

bool piw::xxcontrolled_t::attach_to(controller_t *c,unsigned n)
{
    if(n==0)
    {
        return false;
    }

    if(c!=controller_ || index_!=n)
    {
        detach();

        if(!c->impl_->attach_to(this,n-1))
        {
            return false;
        }

        index_=n-1;
        controller_=c;
        piw::tsd_fastcall(__init,this,0);
    }

    return true;
}

void piw::xxcontrolled_t::attach(controller_t *c)
{
    if(c!=controller_)
    {
        detach();
        index_=c->impl_->attach(this);
        controller_=c;
        piw::tsd_fastcall(__init,this,0);
    }
}

int piw::xxcontrolled_t::ordinal()
{
    return controller_?(int)index_:-1;
}

void piw::xxcontrolled_t::detach()
{
    if(controller_)
    {
        controller_->impl_->set_light(index_,0);
        controller_->impl_->detach(index_);
        controller_=0;
    }
}

void piw::xxcontrolled_t::set_light(unsigned index, unsigned value)
{
    if(controller_)
        controller_->impl_->set_light(index,value);
}

void piw::xxcontrolled_t::save_lights()
{
    if(controller_)
        controller_->impl_->save_lights();
}

void piw::xxcontrolled_t::restore_lights()
{
    if(controller_)
        controller_->impl_->restore_lights();
}

piw::fastdata_t *piw::xxcontrolled_t::fastdata()
{
    return xximpl_->fastdata();
}

piw::change_nb_t piw::xxcontrolled_t::sender()
{
    return xximpl_->sender();
}

void piw::xxcontrolled_t::send(const piw::data_nb_t &d)
{
    xximpl_->send(d);
}

void piw::xxcontrolled_t::start_event(unsigned long long t)
{
    xximpl_->start_event(t);
}

void piw::xxcontrolled_t::end_event(unsigned long long t)
{
    xximpl_->end_event(t);
}


piw::cookie_t piw::controller_t::cookie()
{
    return impl_->filter_.cookie();
}

static int __start(void *c_, void *)
{
    piw::controlled_t *c = (piw::controlled_t *)c_;
    unsigned long long t = piw::tsd_time();
    c->start_event(t);
    return 0;
}

static int __term(void *c_, void *)
{
    piw::controlled_t *c = (piw::controlled_t *)c_;
    unsigned long long t = piw::tsd_time();
    c->end_event(t);
    return 0;
}

piw::controlled_t::controlled_t(bool enabled): enabled_(enabled)
{
    piw::tsd_fastcall(__start,this,0);
}

piw::controlled_t::~controlled_t()
{
    piw::tsd_fastcall(__term,this,0);
}

void piw::controlled_t::control_init()
{
    set_light(ordinal(),enabled_?1:3);
}

int piw::controlled_t::__enable_direct(void *c_, void *_)
{
    piw::controlled_t *c = (piw::controlled_t *)c_;
    c->control_enable();
    c->control_init();
    return 0;
}

int piw::controlled_t::__disable_direct(void *c_, void *_)
{
    piw::controlled_t *c = (piw::controlled_t *)c_;
    c->control_disable();
    c->control_init();
    return 0;
}

void piw::controlled_t::control_receive(unsigned index,const piw::data_nb_t &value)
{
    if(index!=0) return;
    if(!value.is_array()) return;
    if(value.as_arraylen()==0 || value.as_renorm(0.0,3.0,0.0)<2) return;

    if(enabled_)
        disable();
    else
        enable();
}

void piw::controlled_t::enable()
{
    if(!enabled_)
    {
        enabled_=true;
        tsd_fastcall(__enable_direct,this,0);
    }
}

void piw::controlled_t::disable()
{
    if(enabled_)
    {
        enabled_=false;
        tsd_fastcall(__disable_direct,this,0);
    }
}

piw::fasttrigger_t::fasttrigger_t(unsigned color): color_(color), active_(0)
{
}

void piw::fasttrigger_t::control_init()
{
    set_light(ordinal(),color_);
}

int piw::fasttrigger_t::__ping_direct(void *c_, void *v_)
{
    unsigned long long t;
    piw::fasttrigger_t *c = (piw::fasttrigger_t *)c_;

    if(v_)
    {
        t = *(unsigned long long *)v_;
    }
    else
    {
        t = piw::tsd_time();
    }

    pic::logmsg() << "trigger fired " << t;
    c->send(piw::makebool_nb(true,t));
    c->control_start(t);
    c->send(piw::makebool_nb(false,t+1));
    c->control_term(t+2);
    return 0;
}

void piw::fasttrigger_t::control_start(unsigned long long t)
{
    if(active_++==0)
    {
        start_event(t);
    }
}

void piw::fasttrigger_t::control_term(unsigned long long t)
{
    if(active_>0)
    {
        if(--active_==0)
        {
            end_event(t);
        }
    }
}

void piw::fasttrigger_t::control_receive(unsigned index,const piw::data_nb_t &value)
{
    if(index!=0) return;
    if(!value.is_array()) return;
    if(value.as_arraylen()==0 || value.as_renorm(0.0,3.0,0.0)<2) return;
    unsigned long long t = value.time();
    tsd_fastcall(__ping_direct,this,&t);
}

void piw::fasttrigger_t::trigger__(const piw::data_nb_t &d)
{
    if(d.as_norm()!=0)
    {
        unsigned long long t = d.time();
        tsd_fastcall(__ping_direct,this,&t);
    }
}

void piw::fasttrigger_t::ping()
{
    tsd_fastcall(__ping_direct,this,0);
}
