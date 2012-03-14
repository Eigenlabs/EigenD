
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
#include <piw/piw_keys.h>
#include <piw/piw_thing.h>

#include <picross/pic_log.h>
#include <picross/pic_flipflop.h>

#include <vector>
#include <map>
#include <set>


namespace
{
    typedef std::set<piw::xxcontrolled_t *> ctlset_t;

    struct ctlfilter_t: piw::ufilterfunc_t
    {
        ctlfilter_t(piw::controller_t::impl_t *controller): controller_(controller) {}

        void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id);
        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d);
        void ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long);
        void ufilterfunc_changed(piw::ufilterenv_t *env, void *c) { erase_controlled((piw::xxcontrolled_t *)c); }

        void start__(unsigned long long);
        void end__(unsigned long long);
        void erase_controlled(piw::xxcontrolled_t *);

        piw::controller_t::impl_t *controller_;
        piw::dataholder_nb_t id_;
        ctlset_t controlled_;
        piw::dataholder_nb_t current_key_,current_ctl_;
    };

    static int __init(void *xctl_, void *)
    {
        piw::xxcontrolled_t *xctl = (piw::xxcontrolled_t *)xctl_;
        xctl->control_init();
        return 0;
    }

    static bool compare_phys_key(const piw::data_nb_t &k1, const piw::data_nb_t &k2)
    {
        unsigned k1n; float k1r,k1c;
        unsigned k2n; float k2r,k2c;

        if(!piw::decode_key(k1,&k1n,&k1r,&k1c)) return false;
        if(!piw::decode_key(k2,&k2n,&k2r,&k2c)) return false;
        if(k1n!=k2n) return false;
        if(k1r!=k2r) return false;
        if(k1c!=k2c) return false;
        return true;
    }
}

struct piw::controller_t::ctlsignal_t: piw::wire_ctl_t, piw::event_data_source_real_t
{
    ctlsignal_t(piw::xxcontrolled_t *c, unsigned id): piw::event_data_source_real_t(piw::pathone(id,0)), id_(id), client_(c), state_(0), savestate_(0), buffer_(piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_TINY))
    {
    }

    ~ctlsignal_t()
    {
        source_shutdown();
    }

    void save()
    {
        savestate_=state_;
        set(0);
    }

    void restore()
    {
        set(savestate_);
    }

    void source_ended(unsigned seq)
    {
    }

    void set__(unsigned v)
    {
        if(v==state_)
        {
            return;
        }

        if(v)
        {
            unsigned t = piw::tsd_time();
            buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,v,t));

            if(!state_)
            {
                {
                    pic::flipflop_t<piw::data_t>::guard_t gk(key_);
                    if(gk.value().is_tuple())
                    {
                        buffer_.add_value(2,gk.value().make_nb().restamp(t));
                    }
                }
                source_start(0,piw::pathone_nb(id_,t),buffer_);
            }
        }
        else
        {
            source_end(piw::tsd_time());
        }

        state_=v;
    }

    static int reset0__(void *self_, void *)
    {
        ctlsignal_t *self = (ctlsignal_t *)self_;
        unsigned os = self->state_;
        self->set__(0);
        self->set__(os);
        return 1;
    }

    static int set0__(void *self_, void *v_)
    {
        ctlsignal_t *self = (ctlsignal_t *)self_;
        unsigned v = *(unsigned *)v_;
        self->set__(v);
        return 1;
    }

    void set(unsigned v)
    {
        piw::tsd_fastcall(set0__,this,&v);
    }


    void set_key(const piw::data_t &key)
    {
        key_.set(key);
        piw::tsd_fastcall(reset0__,this,0);
    }

    unsigned id_;
    piw::xxcontrolled_t *client_;
    unsigned state_,savestate_;
    pic::flipflop_t<piw::data_t> key_;
    piw::xevent_data_buffer_t buffer_;
};

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

    void get_controlled(ctlset_t &ctlset, const piw::data_nb_t &key, const piw::data_nb_t &ctl)
    {
        ctlset.clear();

        unsigned kn = 0;
        float kr = 0;
        float kc = 0;

        if(!piw::decode_key(key,&kn,&kr,&kc)) return;
        if(kr==0 && kc==0) { return; }

        piw::data_nb_t rowlen;
        int nr = 0;

        if(ctl.is_dict())
        {
            piw::data_nb_t t=ctl.as_dict_lookup("rowlen");

            if(t.is_tuple() && t.as_tuplelen()>0)
            {
                rowlen=t;
                nr = t.as_tuplelen();
            }
        }

        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t lg(controllist_);
        const std::vector<ctlsignal_t *> &l(lg.value());
        std::vector<ctlsignal_t *>::const_iterator li;

        for(li=l.begin();li!=l.end();li++)
        {
            if((*li)==0)
            {
                continue;
            }

            pic::flipflop_t<piw::data_t>::guard_t g((*li)->key_);

            if(!g.value().is_tuple())
            {
                continue;
            }

            int r = g.value().as_tuple_value(0).as_long();
            int c = g.value().as_tuple_value(1).as_long();

            if(r==0)
            {
                if(unsigned(c)==kn)
                {
                    ctlset.insert((*li)->client_);
                    continue;
                }
            }

            if(r<0)
            {
                if(!nr) continue;
                r = r+nr+1;
                if(r<=0 || r>nr) continue;
            }

            if(r!=kr) continue;

            if(c<0)
            {
                if(!nr) continue;
                if(r<=0 || r>nr) continue;
                int nc = rowlen.as_tuple_value(r-1).as_long();
                c = c+nc+1;
                if(c<=0 || c>nc) continue;
            }

            if(c!=kc) continue;

            ctlset.insert((*li)->client_);
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

    void set_key(unsigned index, const piw::data_t &key)
    {
        if(!key.is_tuple() || key.as_tuplelen() != 2) return;

        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t gc(controllist_);

        if(index<gc.value().size())
        {
            ctlsignal_t *c = gc.value()[index];

            if(c)
            {
                c->set_key(key);
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
        filter_.changed(s->client_);
        delete s;
    }

    piw::ufilter_t filter_;
    pic::flipflop_t<std::vector<ctlsignal_t *> > controllist_;
    unsigned long sigmask_;
    piw::controller_t *host_;
};

void ctlfilter_t::erase_controlled(piw::xxcontrolled_t *c)
{
    controlled_.erase(c);
}

void ctlfilter_t::end__(unsigned long long t)
{
    ctlset_t::const_iterator it;

    for(it=controlled_.begin(); it!=controlled_.end(); ++it)
    {
        (*it)->control_term(t);
    }
}

void ctlfilter_t::ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long t)
{
    if(!id_.is_empty())
    {
        end__(t);
        id_.clear();
    }
}

void ctlfilter_t::start__(unsigned long long t)
{
    controller_->get_controlled(controlled_,current_key_.get(),current_ctl_.get());

    ctlset_t::const_iterator it;

    for(it=controlled_.begin(); it!=controlled_.end(); ++it)
    {
        (*it)->control_start(t);
    }
}

void ctlfilter_t::ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
{
    piw::data_nb_t k,c;

    id_.set_nb(id);

    env->ufilterenv_latest(1,k,id.time());
    env->ufilterenv_latest(2,c,id.time());

    current_key_.set_nb(k);
    current_ctl_.set_nb(c);

    env->ufilterenv_start(id.time());

    start__(id.time());
}

void ctlfilter_t::ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d)
{
    if(!id_.is_empty())
    {
        bool changed = false;

        if(s==1)
        {
            if(!compare_phys_key(d,current_key_.get()))
            {
                current_key_.set_nb(d);
                changed=true;
            }
        }

        if(s==2)
        {
            if(d.compare(current_ctl_.get(),false)!=0)
            {
                current_ctl_.set_nb(d);
                changed=true;
            }
        }

        if(changed)
        {
            end__(d.time());
            start__(d.time());
            return;
        }

        ctlset_t::const_iterator it;

        for(it=controlled_.begin(); it!=controlled_.end(); ++it)
        {
            (*it)->control_receive(s,d);
        }
    }
}

piw::controller_t::controller_t(const piw::cookie_t &c,const std::string &sigmap): impl_(new impl_t(this,c,sigmap))
{
}

piw::controller_t::~controller_t()
{
    delete impl_;
}

piw::xxcontrolled_t::xxcontrolled_t() : controller_(0), xximpl_(new impl_t())
{
}

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

void piw::xxcontrolled_t::set_key(const piw::data_t &key)
{
    if(controller_)
        controller_->impl_->set_key(index_,key);
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


piw::cookie_t piw::controller_t::event_cookie()
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

void piw::controlled_t::control_receive(unsigned s,const piw::data_nb_t &value)
{
    if(s!=1) return;

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
    c->start_event(t);
    c->send(piw::makebool_nb(false,t+1));
    c->end_event(t+2);

    return 0;
}

void piw::fasttrigger_t::control_start(unsigned long long t)
{
}

void piw::fasttrigger_t::control_term(unsigned long long t)
{
}

void piw::fasttrigger_t::control_receive(unsigned s,const piw::data_nb_t &value)
{
    piw::hardness_t h;

    if(s==1 && piw::decode_key(value,0,0,0,0,0,0,&h) && h!=piw::KEY_LIGHT)
    {
        unsigned long long t = value.time();
        __ping_direct(this,&t);
    }
}

void piw::fasttrigger_t::trigger__(const piw::data_t &d)
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
