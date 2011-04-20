
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

#include <piw/piw_mixer.h>
#include <piw/piw_clock.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_tsd.h>

#include <picross/pic_float.h>

#define CLKSTATE_IDLE 0
#define CLKSTATE_SHUTDOWN 1
#define CLKSTATE_RUNNING 2
#define CLKSTATE_STARTUP 3

namespace
{
    struct stereomixer_ctlwire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t
    {
        stereomixer_ctlwire_t(piw::stereomixer_t::impl_t *impl);
        ~stereomixer_ctlwire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);
        void ticked(unsigned long long t);

        piw::stereomixer_t::impl_t *impl_;
        piw::dataqueue_t vinput_,pinput_;
        unsigned long long vindex_,pindex_;
    };

    struct stereomixer_audiowire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t, pic::element_t<>
    {
        stereomixer_audiowire_t(piw::stereomixer_t::impl_t *impl);
        ~stereomixer_audiowire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned seq,const piw::data_nb_t &d,const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void ticked(unsigned long long t);

        piw::stereomixer_t::impl_t *impl_;
        unsigned long long lindex_,rindex_;
        piw::dataqueue_t linput_,rinput_;
    };
};

struct piw::stereomixer_t::impl_t: piw::root_t, piw::root_ctl_t, piw::wire_ctl_t, piw::clocksink_t, piw::event_data_source_real_t, virtual public pic::lckobject_t
{
    impl_t(const pic::f2f_t &, const pic::f2f_t &,piw::clockdomain_ctl_t *d, const piw::cookie_t &c);
    ~impl_t() { tracked_invalidate(); invalidate(); }
    void invalidate();

    piw::wire_t *root_wire(const piw::event_data_source_t &es);

    void root_clock();
    void root_latency() { set_sink_latency(get_latency()); set_latency(get_latency()); }
    void root_opened();
    void root_closed() { }

    void source_ended(unsigned seq) { }

    void clocksink_ticked(unsigned long long from,unsigned long long t);
    piw::data_nb_t mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float adj,unsigned long long, unsigned bs);

    void setadj();

    bct_clocksink_t *up_;
    std::list<stereomixer_audiowire_t *> inputs_;
    int state_;

    pic::lcklist_t<piw::data_nb_t>::nbtype lqueue_;
    pic::lcklist_t<piw::data_nb_t>::nbtype rqueue_;

    float ladj_, radj_, volctl_, lpanctl_, rpanctl_;

    pic::flipflop_functor_t<pic::f2f_t> volfunc_;
    pic::flipflop_functor_t<pic::f2f_t> panfunc_;
    stereomixer_ctlwire_t ctlwire_;
    pic::ilist_t<stereomixer_audiowire_t> active_;
    piw::xevent_data_buffer_t buffer_;
};

stereomixer_ctlwire_t::stereomixer_ctlwire_t(piw::stereomixer_t::impl_t *impl): impl_(impl)
{
}

stereomixer_audiowire_t::stereomixer_audiowire_t(piw::stereomixer_t::impl_t *impl): impl_(impl)
{
    impl_->inputs_.push_back(this);
}

void stereomixer_ctlwire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    piw::data_nb_t d;

    if(s==1)
    {
        vinput_=n;
        vindex_=0;
        vinput_.latest(d,&vindex_,t);
    }
    
    if(s==2)
    {
        pinput_=n;
        pindex_=0;
        pinput_.latest(d,&pindex_,t);
    }
}

void stereomixer_ctlwire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei)
{
    piw::data_nb_t d;

    vinput_=ei.signal(1);
    vindex_=0;
    if(vinput_.latest(d,&vindex_,id.time()))
    {
        vindex_++;
        impl_->volctl_ = d.as_renorm(0,120,0);
        impl_->setadj();
    }

    pinput_=ei.signal(2);
    pindex_=0;
    if(pinput_.latest(d,&pindex_,id.time()))
    {
        pindex_++;
        float u = d.as_array_ubound();
        float l = d.as_array_lbound();
        float r = d.as_array_rest();

        impl_->lpanctl_ = piw::denormalise(u,l,r,-d.as_norm());
        impl_->rpanctl_ = piw::denormalise(u,l,r,d.as_norm());
        impl_->setadj();
    }
}

void stereomixer_ctlwire_t::ticked(unsigned long long t)
{
    piw::data_nb_t d;
    if(vinput_.latest(d,&vindex_,t))
    {
        vindex_++;
        impl_->volctl_ = d.as_renorm(0,120,0);
        impl_->setadj();
    }

    if(pinput_.latest(d,&pindex_,t))
    {
        pindex_++;
        float u = d.as_array_ubound();
        float l = d.as_array_lbound();
        float r = d.as_array_rest();

        impl_->lpanctl_ = piw::denormalise(u,l,r,-d.as_norm());
        impl_->rpanctl_ = piw::denormalise(u,l,r,d.as_norm());
        impl_->setadj();
    }
}

bool stereomixer_ctlwire_t::event_end(unsigned long long t)
{
    return true;
}

void stereomixer_audiowire_t::event_start(unsigned seq,const piw::data_nb_t &eid,const piw::xevent_data_buffer_t &ei)
{
    piw::data_nb_t d;

    linput_=ei.signal(1);
    lindex_=0;
    if(linput_.latest(d,&lindex_,eid.time()))
    {
        ++lindex_;
        impl_->lqueue_.push_back(d);
    }

    rinput_=ei.signal(2);
    rindex_=0;
    if(rinput_.latest(d,&rindex_,eid.time()))
    {
        ++rindex_;
        impl_->rqueue_.push_back(d);
    }

    if(!impl_->active_.head())
    {
        impl_->tick_suppress(false);
        impl_->state_ = CLKSTATE_STARTUP;
    }

    impl_->active_.append(this);
}

void stereomixer_audiowire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    piw::data_nb_t d;

    if(s==1)
    {
        linput_=n;
        lindex_=0;
        linput_.latest(d,&lindex_,t);
    }

    if(s==2)
    {
        rinput_=n;
        rindex_=0;
        rinput_.latest(d,&rindex_,t);
    }
}

void stereomixer_audiowire_t::ticked(unsigned long long t)
{
    piw::data_nb_t d;
    while(linput_.read(d,&lindex_,t))
    {
        lindex_++;
        impl_->lqueue_.push_back(d);
    }

    while(rinput_.read(d,&rindex_,t))
    {
        rindex_++;
        impl_->rqueue_.push_back(d);
    }
}

bool stereomixer_audiowire_t::event_end(unsigned long long t)
{
    ticked(t);
    remove();
    return true;
}

void stereomixer_ctlwire_t::invalidate()
{
    unsubscribe();
    wire_t::disconnect();
}

void stereomixer_audiowire_t::invalidate()
{
    unsubscribe();
    wire_t::disconnect();
    impl_->inputs_.remove(this);
}

piw::stereomixer_t::impl_t::impl_t(const pic::f2f_t &v, const pic::f2f_t &p, piw::clockdomain_ctl_t *d, const piw::cookie_t &c): root_t(0), piw::event_data_source_real_t(piw::pathnull(0)), up_(0), state_(CLKSTATE_SHUTDOWN), volctl_(120), lpanctl_(0),rpanctl_(0),volfunc_(v), panfunc_(p), ctlwire_(this), buffer_(3,PIW_DATAQUEUE_SIZE_ISO)
{
    setadj();

    d->sink(this,"stereomixer");
    set_clock(this);

    connect(c);
    connect_wire(this,source());

    tick_enable(true);
}

piw::wire_t *piw::stereomixer_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    piw::data_t path(es.path());
    if(path.as_pathlen()>0)
    {
        if(path.as_path()[0]==1)
        {
            ctlwire_.subscribe(es);
            return &ctlwire_;
        }

        stereomixer_audiowire_t *w= new stereomixer_audiowire_t(this);
        w->subscribe(es);
        return w;
    }

    return 0;
}

void piw::stereomixer_t::impl_t::root_clock()
{
    bct_clocksink_t *s(get_clock());

    if(up_)
    {
        remove_upstream(up_);
        up_ = 0;
    }
    if(s)
    {
        up_ = s;
        add_upstream(s);
    }
}

void piw::stereomixer_t::impl_t::root_opened()
{
    root_clock();
    root_latency();
}

void piw::stereomixer_t::impl_t::invalidate()
{
    std::list<stereomixer_audiowire_t *>::iterator i;

    source_shutdown();

    while((i=inputs_.begin())!=inputs_.end())
    {
        delete (*i);
    }

    ctlwire_.invalidate();
}

piw::data_nb_t piw::stereomixer_t::impl_t::mix(pic::lcklist_t<piw::data_nb_t>::nbtype &queue, float adj, unsigned long long t, unsigned bs)
{
    float *f,*fs;
    piw::data_nb_t d = piw::makenorm_nb(t,bs,&f,&fs);
    memset(f,0,bs*sizeof(float));

    while(queue.size()>1)
    {
        const float *df = queue.front().as_array();
        unsigned dfl = std::min(bs,queue.front().as_arraylen());
        pic::vector::vectadd(df,1,f,1,f,1,dfl);
        queue.pop_front();
    }

    if(queue.size()==1)
    {
        const float *df = queue.front().as_array();
        unsigned dfl = std::min(bs,queue.front().as_arraylen());
        pic::vector::vectasm(df,1,f,1,&adj,f,1,dfl);
        queue.pop_front();
    }

    *fs=f[bs-1];
    return d;
}

void piw::stereomixer_t::impl_t::setadj()
{
    float v = volfunc_(volctl_);

    ladj_=v*panfunc_(lpanctl_);
    radj_=v*panfunc_(rpanctl_);
}


void piw::stereomixer_t::impl_t::clocksink_ticked(unsigned long long from,unsigned long long t)
{
    stereomixer_audiowire_t *w=active_.head();
    if(!w)
    {
        state_=CLKSTATE_SHUTDOWN;
        tick_suppress(true);
    }

    while(w)
    {
        w->ticked(t);
        w=active_.next(w);
    }

    unsigned bs = get_buffer_size();
    ctlwire_.ticked(t);

    buffer_.add_value(1,mix(lqueue_,ladj_,t,bs));
    buffer_.add_value(2,mix(rqueue_,radj_,t,bs));

    if(state_==CLKSTATE_STARTUP)
    {
        state_=CLKSTATE_RUNNING;
        source_start(0,piw::pathnull_nb(t),buffer_);
    }

    if(state_==CLKSTATE_SHUTDOWN)
    {
        source_end(t);
        state_ = CLKSTATE_IDLE;
        buffer_ = piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_ISO);
        tick_suppress(true);
    }
}

piw::stereomixer_t::stereomixer_t(const pic::f2f_t &vol, const pic::f2f_t &pan, piw::clockdomain_ctl_t *d, const piw::cookie_t &c) : impl_(new impl_t(vol,pan,d,c))
{
}

piw::stereomixer_t::~stereomixer_t()
{
    delete impl_;
}

piw::cookie_t piw::stereomixer_t::cookie()
{
    return piw::cookie_t(impl_);
}

int piw::stereomixer_t::gc_traverse(void *v,void *a) const
{
    int r;
    if((r=impl_->volfunc_.gc_traverse(v,a))!=0) return r;
    if((r=impl_->panfunc_.gc_traverse(v,a))!=0) return r;
    return 0;
}

int piw::stereomixer_t::gc_clear()
{
    impl_->volfunc_.gc_clear();
    impl_->panfunc_.gc_clear();
    return 0;
}
