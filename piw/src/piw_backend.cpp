
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

#include <piw/piw_backend.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_flipflop.h>

namespace
{
    struct btof_fast_t: virtual public pic::lckobject_t, virtual public pic::atomic_counted_t
    {
        btof_fast_t(unsigned signal): signal_(signal)
        {
            send_duplicates_.alternate()=false;
            send_duplicates_.exchange();
        }

        void send(const piw::data_nb_t &p, const piw::data_nb_t &d)
        {
            //pic::logmsg() << "backend " << p << " recv data " << d;
            std::map<piw::data_t,piw::change_nb_t,piw::grist_less>::const_iterator fi;

            {
                pic::flipflop_t<std::map<piw::data_t,piw::change_nb_t,piw::grist_less> >::guard_t g(functors_);

                if((fi=g.value().find(p.make_normal())) != g.value().end())
                {
                    fi->second(d);
                }
            }

            {
                pic::flipflop_t<piw::change_nb_t>::guard_t g2(gfunctor_);
                g2.value()(d);
            }

            if(!d.is_null() && d.as_norm()!=0.0)
            {
                pic::flipflop_t<piw::change_nb_t>::guard_t g2(efunctor_);
                g2.value()(p);
            }
        }

        int gc_traverse(void *v, void *a)
        {
            const std::map<piw::data_t,piw::change_nb_t,piw::grist_less> &l(functors_.current());
            std::map<piw::data_t,piw::change_nb_t,piw::grist_less>::const_iterator i;
            int r;

            for(i=l.begin();i!=l.end();i++)
            {
                if((r=i->second.gc_traverse(v,a))!=0)
                {
                    return r;
                }
            }

            if((r=efunctor_.current().gc_traverse(v,a))!=0)
            {
                return r;
            }

            if((r=gfunctor_.current().gc_traverse(v,a))!=0)
            {
                return r;
            }

            return 0;
        }

        int gc_clear()
        {
            efunctor_.set(piw::change_nb_t());
            gfunctor_.set(piw::change_nb_t());
            functors_.alternate().clear();
            functors_.exchange();
            return 0;
        }

        unsigned signal_;
        pic::flipflop_t<std::map<piw::data_t, piw::change_nb_t,piw::grist_less> > functors_;
        pic::flipflop_t<piw::change_nb_t> efunctor_;
        pic::flipflop_t<piw::change_nb_t> gfunctor_;
        pic::flipflop_t<bool> send_duplicates_;
    };

    struct btof_wire_t: piw::wire_t, piw::event_data_sink_t, virtual public pic::lckobject_t, piw::fastdata_t
    {
        btof_wire_t(const piw::event_data_source_t &es, piw::functor_backend_t::impl_t *i);
        ~btof_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_data(const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);

        bool fastdata_receive_event(const piw::data_nb_t &id, const piw::dataqueue_t &q);

        bool fastdata_receive_data(const piw::data_nb_t &d)
        {
            if(fast_->send_duplicates_.current() || d.compare(last_.get(),false)!=0)
            {
                last_.set_nb(d);
                fast_->send(id_,d);
            }

            return true;
        }

        piw::data_t path_;
        piw::functor_backend_t::impl_t *root_;
        piw::dataholder_nb_t id_;
        pic::ref_t<btof_fast_t> fast_;
        piw::dataholder_nb_t last_;
    };
}

struct piw::functor_backend_t::impl_t: piw::decode_ctl_t
{
    impl_t(unsigned signal,bool sn): decoder_(this),send_null_(sn)
    {
        fast_=pic::ref(new btof_fast_t(signal));
    }

    ~impl_t()
    {
    }

    void set_clock(bct_clocksink_t *c) { }
    void set_latency(unsigned l) {}

    wire_t *wire_create(const piw::event_data_source_t &es)
    {
        std::map<piw::data_t, btof_wire_t *>::iterator wi =  wires_.find(es.path());

        if(wi != wires_.end())
            delete wi->second;

        return new btof_wire_t(es,this);
    }

    void set_gfunctor(const piw::change_nb_t &f)
    {
        fast_->gfunctor_.alternate()=f;
        fast_->gfunctor_.exchange();
    }

    void clear_gfunctor()
    {
        fast_->gfunctor_.alternate().clear();
        fast_->gfunctor_.exchange();
    }

    void set_efunctor(const piw::change_nb_t &f)
    {
        fast_->efunctor_.alternate()=f;
        fast_->efunctor_.exchange();
    }

    void clear_efunctor()
    {
        fast_->efunctor_.alternate().clear();
        fast_->efunctor_.exchange();
    }

    void set_functor(const bct_data_t d, const piw::change_nb_t &f)
    {
        tsd_fastcall3(set_functor__,this,(void*)&d,(void*)&f);
    }

    static int set_functor__(void *p1, void *p2, void *p3)
    {
        impl_t *self = (impl_t *)p1;
        bct_data_t d = *(bct_data_t *)p2;
        const piw::change_nb_t f = *(const piw::change_nb_t *)p3;
        
        self->fast_->functors_.alternate().insert(std::make_pair(piw::data_t::from_given(d),f));
        self->fast_->functors_.exchange();

        return 1;
    }

    void clear_functor(const bct_data_t d)
    {
        tsd_fastcall(clear_functor__,this,(void*)&d);
    }

    static int clear_functor__(void *p1, void *p2)
    {
        impl_t *self = (impl_t *)p1;
        bct_data_t d = *(bct_data_t *)p2;

        self->fast_->functors_.alternate().erase(piw::data_t::from_given(d));
        self->fast_->functors_.exchange();

        return 1;
    }

    void send_duplicates(bool state)
    {
        fast_->send_duplicates_.alternate()=state;
        fast_->send_duplicates_.exchange();
    }

    std::map<piw::data_t, btof_wire_t *> wires_;
    piw::decoder_t decoder_;
    pic::ref_t<btof_fast_t> fast_;
    bool send_null_;
};

btof_wire_t::btof_wire_t(const piw::event_data_source_t &es, piw::functor_backend_t::impl_t *i) : fastdata_t(PLG_FASTDATA_SENDER), path_(es.path()), root_(i), fast_(root_->fast_)
{
    piw::tsd_fastdata(this);
    root_->wires_.insert(std::make_pair(path_,this));
    subscribe(es);
    enable(true,false,false);
}

bool btof_wire_t::fastdata_receive_event(const piw::data_nb_t &id, const piw::dataqueue_t &q)
{
    if(!id.is_null())
    {
        last_.clear();
        id_.set_nb(id);
        ping(id.time(),q);
        return true;
    }
    if(root_->send_null_)
        fastdata_receive_data(piw::data_nb_t());
    return false;
}

void btof_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    //pic::logmsg() << "backend event start " << id;
    send_fast(id,b.signal(fast_->signal_));
    id_.set_nb(id);
}

void btof_wire_t::event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(sig==fast_->signal_)
        send_fast(current_id(),n);
}

bool btof_wire_t::event_end(unsigned long long t)
{
    //pic::logmsg() << "backend event end";
    send_fast(piw::data_nb_t(),piw::dataqueue_t());
    return true;
}

void btof_wire_t::invalidate()
{
    unsubscribe();

    if(root_)
    {
        root_->wires_.erase(path_);
        root_ = 0;
    }
}

piw::functor_backend_t::functor_backend_t(unsigned signal,bool send_null) : impl_(new impl_t(signal,send_null)) {}
piw::functor_backend_t::~functor_backend_t() { delete impl_; }
void piw::functor_backend_t::set_functor(const piw::data_t &p, const piw::change_nb_t &f) { impl_->set_functor(p.give_copy(PIC_ALLOC_NB),f); }
void piw::functor_backend_t::clear_functor(const piw::data_t &p) { impl_->clear_functor(p.give_copy(PIC_ALLOC_NB)); }
void piw::functor_backend_t::set_gfunctor(const piw::change_nb_t &f) { impl_->set_gfunctor(f); }
void piw::functor_backend_t::clear_gfunctor() { impl_->clear_gfunctor(); }
void piw::functor_backend_t::set_efunctor(const piw::change_nb_t &f) { impl_->set_efunctor(f); }
void piw::functor_backend_t::clear_efunctor() { impl_->clear_efunctor(); }
piw::cookie_t piw::functor_backend_t::cookie() { return impl_->decoder_.cookie(); }
bct_clocksink_t *piw::functor_backend_t::get_clock() { return impl_->decoder_.get_clock(); }
void piw::functor_backend_t::send_duplicates(bool b) { impl_->send_duplicates(b); }
int piw::functor_backend_t::gc_traverse(void *v, void *a) const { return impl_->fast_->gc_traverse(v,a); }
int piw::functor_backend_t::gc_clear() { return impl_->fast_->gc_clear(); }
