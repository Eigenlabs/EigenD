
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

#include <piw/piw_ufilter.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>

#include <picross/pic_log.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_fastalloc.h>

#include <vector>
#include <map>
#include <cstdio>

namespace
{
    struct filter_wire_t;

    struct filter_holder_t
    {
        filter_holder_t(piw::ufilterctl_t *f,const piw::data_t &path): ctl_(f)
        {
            function_=ctl_->ufilterctl_create(path);
            inputs_=ctl_->ufilterctl_inputs();
            outputs_=ctl_->ufilterctl_outputs();
            thru_=ctl_->ufilterctl_thru();
        }
        ~filter_holder_t() { invalidate(); }
        void invalidate() { if(function_) ctl_->ufilterctl_delete(function_); function_=0; }
        piw::ufilterfunc_t *operator->() { PIC_ASSERT(function_); return function_; }

        piw::ufilterctl_t *ctl_;
        piw::ufilterfunc_t *function_;
        unsigned long long inputs_;
        unsigned long long outputs_;
        unsigned long long thru_;
    };

    struct filter_signal_t: piw::fastdata_t, virtual public pic::lckobject_t
    {
        filter_signal_t(filter_wire_t *w,unsigned signal): fastdata_t(PLG_FASTDATA_SENDER), wire_(w), signal_(signal)
        {
            piw::tsd_fastdata(this);
            enable(true,false,false);
        }

        bool fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q)
        {

            if(!d.is_null())
            {
                //pic::logmsg() << "scaler signal " << signal_ << " input on id " << d;
                //q.dump(false);
                ping(d.time(),q);
                return true;
            }

            return false;
        }

        bool fastdata_receive_data(const piw::data_nb_t &d);

        filter_wire_t *wire_;
        unsigned signal_;
    };

    struct filter_wire_t: piw::event_data_sink_t, piw::event_data_source_real_t, piw::wire_t, piw::ufilterenv_t, piw::wire_ctl_t, virtual public pic::lckobject_t
    {
        filter_wire_t(piw::ufilter_t::impl_t *p, const piw::event_data_source_t &es);
        virtual ~filter_wire_t();
        void wire_closed();
        void changed(void *);
        void invalidate();

        void ufilterenv_output(unsigned signal, const piw::data_nb_t &data);
        void ufilterenv_start(unsigned long long time);
        bool ufilterenv_latest(unsigned,piw::data_nb_t &,unsigned long long);
        void ufilterenv_dump(bool);

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &init);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        void source_ended(unsigned seq);

        piw::ufilter_t::impl_t *parent_;
        piw::data_t path_;
        filter_holder_t holder_;
        piw::dataholder_nb_t id_;


        bool started_;
        unsigned seq_;

        pic::lckvector_t<filter_signal_t *>::lcktype signals_;
        piw::xevent_data_buffer_t input_;
        piw::xevent_data_buffer_t output_;
    };
}

bool filter_signal_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    wire_->holder_->ufilterfunc_data(wire_,signal_,d);
    return true;
}

struct piw::ufilter_t::impl_t: piw::root_t, piw::root_ctl_t, virtual public pic::lckobject_t
{
    impl_t(piw::ufilterctl_t *f, const piw::cookie_t &b);
    void root_closed();
    void root_opened();
    void root_clock();
    void root_latency();
    piw::wire_t *root_wire(const piw::event_data_source_t &es);

    void dochanged(void *);
    void changed(void *);
    void invalidate();

    piw::ufilterctl_t *ctl_;
    pic::flipflop_t<std::map<piw::data_t,filter_wire_t *> > children_;
    void *arg_;
};

piw::ufilter_t::impl_t::impl_t(ufilterctl_t *f, const cookie_t &b): root_t(0), ctl_(f)
{
    connect(b);
}

static int __changer(void *f_, void *_)
{
    piw::ufilter_t::impl_t *f = (piw::ufilter_t::impl_t *)f_;
    f->changed(f->arg_);
    return 0;
}

void piw::ufilter_t::impl_t::dochanged(void *arg)
{
    arg_=arg;
    tsd_fastcall(__changer,this,0);
}

void piw::ufilter_t::impl_t::root_clock()
{
    set_clock(get_clock());
    ctl_->ufilterctl_clock(get_clock());
}

void piw::ufilter_t::impl_t::root_opened()
{
    root_clock();
    root_latency();
}

void piw::ufilter_t::impl_t::root_closed()
{
    invalidate();
    root_clock();
    root_latency();
}

void piw::ufilter_t::impl_t::changed(void *a)
{
    pic::flipflop_t<std::map<piw::data_t,filter_wire_t *> >::guard_t g(children_);
    std::map<piw::data_t,filter_wire_t *>::const_iterator ci = g.value().begin();

    for(;ci!=g.value().end();++ci)
    {
        ci->second->changed(a);
    }
}

void piw::ufilter_t::impl_t::invalidate()
{
    std::map<piw::data_t,filter_wire_t *>::iterator ci;

    while((ci=children_.alternate().begin())!=children_.alternate().end())
    {
        delete ci->second;
    }
}

piw::wire_t *piw::ufilter_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
    std::map<piw::data_t,filter_wire_t *>::iterator ci;

    if((ci=children_.alternate().find(es.path()))!=children_.alternate().end())
    {
        delete ci->second;
    }

    return new filter_wire_t(this,es);
}

void piw::ufilter_t::impl_t::root_latency()
{
    set_latency(get_latency()+ctl_->ufilterctl_latency());
    ctl_->ufilterctl_latency_in(get_latency());
}

filter_wire_t::filter_wire_t(piw::ufilter_t::impl_t *p, const piw::event_data_source_t &es): piw::event_data_source_real_t(es.path()), parent_(p), path_(es.path()), holder_(parent_->ctl_,es.path()), started_(false),  output_(holder_.outputs_,PIW_DATAQUEUE_SIZE_NORM)
{
    parent_->children_.alternate().insert(std::make_pair(path_,this));
    parent_->children_.exchange();
    parent_->connect_wire(this,source());
    for(unsigned i=0; i<64; ++i)
    {
        if(holder_.inputs_&(1ULL<<i))
        {
            signals_.push_back(new filter_signal_t(this,i+1));
        }
    }
    subscribe(es);
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
    holder_->ufilterfunc_changed(this,a);
}

void filter_wire_t::invalidate()
{
    if(parent_)
    {
        unsigned n=signals_.size();
        for(unsigned i=0; i<n; ++i)
        {
            delete signals_[i];
        }
        signals_.clear();

        source_shutdown();
        unsubscribe();
        piw::wire_ctl_t::disconnect();
        piw::wire_t::disconnect();
        holder_.invalidate();
        parent_->children_.alternate().erase(path_);
        parent_->children_.exchange();
        parent_=0;
    }
}

void filter_wire_t::ufilterenv_start(unsigned long long time)
{
    output_.merge(input_,holder_.thru_);
    source_start(seq_,id_.get().restamp(time),output_);
    started_ = true;
}

bool filter_wire_t::ufilterenv_latest(unsigned sig,piw::data_nb_t &d,unsigned long long time)
{
    return input_.latest(sig,d,time);
}

void filter_wire_t::ufilterenv_dump(bool full)
{
    input_.dump(full);
}

void filter_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &init)
{
    PIC_ASSERT(id.is_path());
    id_.set_nb(id);
    seq_=seq;

    input_ = init;
    holder_->ufilterfunc_start(this,id);

    {
        piw::tsd_protect_t p;
        unsigned n=signals_.size();
        for(unsigned i=0; i<n; ++i)
        {
            bct_fastdata_host_send_fast(signals_[i],id.lend(),init.signal(signals_[i]->signal_).lend());
        }
    }
}

bool filter_wire_t::event_end(unsigned long long t)
{
	//pic::logmsg() << "ufilter event ending on " << piw::fullprinter_t(id_);
    holder_->ufilterfunc_end(this,t);

    if(!started_)
    {
        return true;
    }

    {
        piw::tsd_protect_t p;
        unsigned n=signals_.size();
        piw::data_nb_t d = piw::makenull_nb(t);
        piw::dataqueue_t q = piw::dataqueue_t();
        for(unsigned i=0; i<n; ++i)
        {
            bct_fastdata_host_send_fast(signals_[i],d.lend(),q.lend());
        }
    }

    bool r = source_end(t);
    if(r)
    {
        started_ = false;
    }

    output_ = piw::xevent_data_buffer_t(holder_.outputs_,PIW_DATAQUEUE_SIZE_NORM);

    return r;
}

void filter_wire_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &q)
{
    if(!started_)
    {
        return;
    }

    unsigned n=signals_.size();

    for(unsigned i=0; i<n; ++i)
    {
        if(signals_[i]->signal_==sig)
        {
            signals_[i]->send_fast(id_,q);
            break;
        }
    }

    if((sig&holder_.thru_)!=0)
    {
        source_buffer_reset(sig,t,o,q);
    }
}

void filter_wire_t::source_ended(unsigned seq)
{
    started_=false;
    event_ended(seq);
}

void filter_wire_t::ufilterenv_output(unsigned signal, const piw::data_nb_t &data)
{
    if(output_.isvalid(signal))
        output_.add_value(signal,data);
}

piw::ufilter_t::ufilter_t(piw::ufilterctl_t *ctl, const piw::cookie_t &o): root_(new impl_t(ctl,o))
{
}

piw::cookie_t piw::ufilter_t::cookie()
{
    return piw::cookie_t(root_);
}

void piw::ufilter_t::changed(void *a)
{
    root_->dochanged(a);
}

piw::ufilter_t::~ufilter_t()
{
    root_->invalidate();
    delete root_;
}


