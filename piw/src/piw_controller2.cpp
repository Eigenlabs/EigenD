
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

#include <piw/piw_controller2.h>

#include <piw/piw_ufilter.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_keys.h>
#include <piw/piw_thing.h>
#include <piw/piw_status.h>
#include <piw/piw_lights.h>

#include <picross/pic_ref.h>
#include <picross/pic_ilist.h>
#include <picross/pic_log.h>
#include <picross/pic_flipflop.h>

#include <vector>
#include <map>
#include <set>

namespace
{
    struct relative_coordinate_t
    {
        relative_coordinate_t(const piw::coordinate_t &coord): coord_(coord), sequential_(false) {}
        relative_coordinate_t(unsigned offset): offset_(offset), sequential_(true) {}
        relative_coordinate_t(): sequential_(false) {}
        relative_coordinate_t &operator=(const relative_coordinate_t &o) { coord_=o.coord_; sequential_=o.sequential_; offset_=o.offset_; return *this; }

        const piw::coordinate_t to_nonsequential(const piw::data_nb_t &geometry) const
        {
            return to_nonsequential(geometry.make_normal());
        }

        const piw::coordinate_t to_nonsequential(const piw::data_t &geometry) const
        {
            if(!sequential_)
            {
                return coord_;
            }

            if(!offset_ || !geometry.is_tuple())
            {
                return piw::coordinate_t();
            }

            unsigned o = offset_-1;
            unsigned c = 0;
            unsigned tl = geometry.as_tuplelen();

            while(c<tl)
            {
                piw::data_t cd = geometry.as_tuple_value(c);
                if(!cd.is_long()) return piw::coordinate_t();
                unsigned cl = cd.as_long();
                if(o<cl) return piw::coordinate_t(c+1,o+1);
                c++; o-= cl;
            }

            return piw::coordinate_t();
        }

        bool is_sequential() const
        {
            return sequential_;
        }

        bool is_valid() const
        {
            if(!sequential_)
            {
                return coord_.is_valid();
            }

            return offset_>0;
        }

        bool equals(int kc, int kr, const piw::data_t &geometry) const
        {
            if(!sequential_)
            {
                return coord_.equals(kc,kr,geometry);
            }

            return to_nonsequential(geometry).equals(kc,kr,geometry);
        }

        bool equals(int kc, int kr, const piw::data_nb_t &geometry) const
        {
            return equals(kc,kr,geometry.make_normal());
        }

        bool operator==(const relative_coordinate_t &o) const
        {
            if(sequential_)
            {
                if(!o.sequential_) return false;
                return offset_==o.offset_;
            }
            else
            {
                if(o.sequential_) return false;
            }

            return coord_==o.coord_;
        }

        bool operator>(const relative_coordinate_t &o) const
        {
            if(sequential_)
            {
                if(!o.sequential_) return true;
                return offset_>o.offset_;
            }
            else
            {
                if(o.sequential_) return false;
            }

            return coord_>o.coord_;
        }

        bool operator<(const relative_coordinate_t &o) const
        {
            if(sequential_)
            {
                if(!o.sequential_) return false;
                return offset_<o.offset_;
            }
            else
            {
                if(o.sequential_) return true;
            }

            return coord_<o.coord_;
        }

        piw::coordinate_t coord_;
        unsigned offset_;
        bool sequential_;
    };

    struct light_t
    {
        light_t(const relative_coordinate_t &coord, unsigned status): coord_(coord), status_(status) {}
        light_t(): status_(0) {}
        light_t &operator=(const light_t &o) { coord_=o.coord_; status_=o.status_; return *this; }

        bool operator<(const light_t &o) const
        {
            if(coord_<o.coord_) return true;
            if(coord_>o.coord_) return false;
            if(status_<o.status_) return true;
            if(status_>o.status_) return false;
            return false;
        }

        relative_coordinate_t coord_;
        bool sequential_;
        unsigned status_;
    };

    typedef pic::lckset_t<light_t>::nbtype lightset_t;
    typedef std::set<piw::controlled_base2_t *> ctlset_t;

    struct layout_t: pic::atomic_counted_t, pic::lckobject_t
    {
        layout_t(const piw::data_t &offsets, const piw::data_t &lengths, const piw::data_t &columns);

        piw::data_t offsets_definition_;
        piw::data_t lengths_definition_;
        piw::data_t columns_definition_;
    };

    typedef pic::ref_t<layout_t> lref_t;

    struct control_manager_subscriber_t
    {
        virtual ~control_manager_subscriber_t() {}
        virtual void control_changed(const piw::data_t &) {}
    };

    struct control_manager_wire_t;

    struct control_manager_t: piw::ufilterctl_t, piw::ufilter_t, virtual pic::lckobject_t
    {
        control_manager_t(const piw::cookie_t &l): ufilter_t(this,l)
        {
        }

        ~control_manager_t()
        {
        }

        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &);
        void ufilterctl_delete(piw::ufilterfunc_t *f);

        unsigned long long ufilterctl_inputs() { return SIG1(1); }
        unsigned long long ufilterctl_outputs() { return SIG1(1); }
        unsigned long long ufilterctl_thru() { return 0ULL; }
        void update_lights(const lightset_t &lights) { lights_=lights; changed(&lights_); }

        void control_changed(const piw::data_t &id);
        void add_subscriber(control_manager_subscriber_t *s) { subscribers_.push_back(s); }
        void del_subscriber(control_manager_subscriber_t *s) { subscribers_.remove(s); }

        lref_t layout(const piw::data_t &id);

        pic::lckmap_t<piw::data_t,lref_t,piw::path_less>::nbtype layout_;
        pic::lckmap_t<piw::data_t,control_manager_wire_t*,piw::path_less>::nbtype func_;
        pic::lcklist_t<control_manager_subscriber_t *>::nbtype subscribers_;
        lightset_t lights_;
    };

    struct controller_event_wire_t: piw::ufilterfunc_t
    {
        controller_event_wire_t(piw::controller2_t::impl_t *controller): controller_(controller) {}

        void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id);
        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d);
        void ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long);
        void ufilterfunc_changed(piw::ufilterenv_t *env, void *c) { erase_controlled((piw::controlled_base2_t *)c); }

        void start__(unsigned long long);
        void end__(unsigned long long);
        void erase_controlled(piw::controlled_base2_t *);

        piw::controller2_t::impl_t *controller_;
        piw::data_t id_;
        ctlset_t controlled_;
        piw::data_t current_key_,current_ctl_;
    };

    struct control_manager_wire_t: piw::ufilterfunc_t
    {
        control_manager_wire_t(control_manager_t *i): impl_(i), env_(0)
        {
        }

        void ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
        {
            PIC_ASSERT(id.is_path());

            id_ = id.make_normal();
            env_ = e;

            impl_->func_.insert(std::make_pair(id_,this));
            e->ufilterenv_start(id.time());
        }

        void ufilterfunc_data(piw::ufilterenv_t *e,unsigned sig,const piw::data_nb_t &d)
        {
            if(sig!=1) return;
            if(!d.is_dict()) return;

            PIC_ASSERT(env_ == e);

            pic::lckmap_t<piw::data_t,lref_t,piw::path_less>::nbtype::iterator li;

            piw::data_nb_t co,cl,cx;

            li = impl_->layout_.find(id_);
            if(li!=impl_->layout_.end()) impl_->layout_.erase(li);

            co = d.as_dict_lookup("courseoffset");
            cl = d.as_dict_lookup("courselen");
            cx = d.as_dict_lookup("columnlen");

            columnlen_ = cx.make_normal();

            if(cx.is_tuple() && co.is_tuple() && cl.is_tuple()) impl_->layout_.insert(std::make_pair(id_,pic::ref(new layout_t(co.make_normal(),cl.make_normal(),columnlen_))));

            update_lights(e);
            impl_->control_changed(id_);
        }

        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
        {
            env_ = 0;

            pic::lckmap_t<piw::data_t,lref_t,piw::path_less>::nbtype::iterator li;
            pic::lckmap_t<piw::data_t,control_manager_wire_t*,piw::path_less>::nbtype::iterator fi;

            li = impl_->layout_.find(id_);
            if(li!=impl_->layout_.end()) impl_->layout_.erase(li);

            fi = impl_->func_.find(id_);
            if(fi!=impl_->func_.end()) impl_->func_.erase(fi);

            id_ = piw::data_t();
        }

        void update_lights(piw::ufilterenv_t *env)
        {
            piw::statusset_t status;

            lightset_t::const_iterator i = impl_->lights_.begin();

            while(i!=impl_->lights_.end())
            {
                status.insert(piw::statusdata_t(false,i->coord_.to_nonsequential(columnlen_),i->status_));
                i++;
            }

            env->ufilterenv_output(1,piw::statusbuffer_t::make_statusbuffer(status));

        }

        void ufilterfunc_changed(piw::ufilterenv_t *env, void *)
        {
            update_lights(env);
        }

        piw::data_t id_;
        control_manager_t *impl_;
        piw::ufilterenv_t *env_;
        lightset_t lights_;
        piw::data_t columnlen_;
    };

    static int __init(void *xctl_, void *)
    {
        piw::controlled_base2_t *xctl = (piw::controlled_base2_t *)xctl_;
        xctl->control_init();
        return 0;
    }

    static bool compare_phys_key(const piw::data_nb_t &k1, const piw::data_t &k2)
    {
        float k1c,k1r;
        float k2c,k2r;

        if(!piw::decode_key(k1,&k1c,&k1r)) return false;
        if(!piw::decode_key(k2,&k2c,&k2r)) return false;
        if(k1c!=k2c) return false;
        if(k1r!=k2r) return false;
        return true;
    }

    class light_receiver_t;

    struct light_receiver_wire_t: piw::ufilterfunc_t, virtual pic::lckobject_t, pic::element_t<0>
    {
        light_receiver_wire_t(light_receiver_t *root): impl_(root), color_(0), channel_(0)
        {
        }

        ~light_receiver_wire_t()
        {
        }

        void ufilterfunc_changed(piw::ufilterenv_t *, void *);
        void ufilterfunc_start(piw::ufilterenv_t *,const piw::data_nb_t &);
        void ufilterfunc_data(piw::ufilterenv_t *,unsigned,const piw::data_nb_t &);
        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long);

        light_receiver_t *impl_;
        unsigned color_;
        unsigned channel_;
    };

    struct light_receiver_list_t: virtual pic::lckobject_t, virtual pic::atomic_counted_t, pic::nocopy_t
    {
        pic::ilist_t<light_receiver_wire_t> list_;
    };

    struct light_receiver_t: virtual pic::lckobject_t, piw::ufilterctl_t, piw::ufilter_t, virtual pic::tracked_t
    {
        light_receiver_t(piw::controller2_t::impl_t *impl): piw::ufilter_t(this,piw::cookie_t(0)), impl_(impl), size_(0), status_(0)
        {
        }

        ~light_receiver_t()
        {
        }

        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &path)
        {
            return new light_receiver_wire_t(this);
        }

        void update_input(unsigned i)
        {
            receiver_changed();
        }

        unsigned fetch_light(unsigned i, unsigned default_status)
        {
            if(i==0)
            {
                return default_status;
            }

            ensure_size(i);

            light_receiver_wire_t *r;
            light_receiver_list_t *l = inputs_[i-1].ptr();

            unsigned m = CLR_ORANGE;
            unsigned c;
            bool f = false;

            for(r=l->list_.head(); r!=0; r=l->list_.next(r))
            {
                c = r->color_;
                f = true;

                if(c==CLR_ORANGE)
                {
                    continue;
                }

                if(m==CLR_ORANGE)
                {
                    m=c;
                    continue;
                }

                if(c==CLR_MIXED)
                {
                    m=c;
                    continue;
                }

                if(m!=c)
                {
                    m=CLR_MIXED;
                }
            }

            if(!f) 
            {
                return default_status;
            }

            switch(m)
            {
                case CLR_MIXED: return BCTSTATUS_UNKNOWN;
                case CLR_OFF: return BCTSTATUS_OFF;
                case CLR_RED: return BCTSTATUS_INACTIVE;
                case CLR_GREEN: return BCTSTATUS_ACTIVE;
            }

            return default_status;
        }

        void receiver_changed();

        void add_input(light_receiver_wire_t *r, unsigned i)
        {
            if(i>0)
            {
                ensure_size(i);
                inputs_[i-1]->list_.append(r);
                update_input(i);
            }
        }

        void del_input(light_receiver_wire_t *r, unsigned i)
        {
            if(i>0)
            {
                ensure_size(i);
                inputs_[i-1]->list_.remove(r);
                update_input(i);
            }
        }

        void ensure_size(unsigned new_size)
        {
            if(size_ >= new_size)
            {
                return;
            }

            inputs_.resize(new_size);

            if(new_size > size_)
            {
                for(unsigned i = size_; i < new_size; i++)
                {
                    inputs_[i] = pic::ref(new light_receiver_list_t);
                }
            }

            size_ = new_size;
        }

        unsigned long long ufilterctl_thru() { return 0; }
        unsigned long long ufilterctl_inputs() { return SIG1(1); }
        unsigned long long ufilterctl_outputs() { return 0; }

        piw::controller2_t::impl_t *impl_;
        unsigned size_;
        unsigned status_;
        pic::lckvector_t< pic::ref_t<light_receiver_list_t> >::lcktype inputs_;
    };

};

struct piw::controller2_t::ctlsignal_t
{
    ctlsignal_t(piw::controlled_base2_t *c, unsigned id): id_(id), client_(c), state_(0), savestate_(0)
    {
    }

    ~ctlsignal_t()
    {
    }

    void save()
    {
        savestate_=state_;
        state_=0;
    }

    void restore()
    {
        state_=savestate_;
    }

    void set_status(unsigned v)
    {
        state_ = v;
    }

    unsigned get_status()
    {
        return state_;
    }

    unsigned get_id()
    {
        return id_;
    }

    const relative_coordinate_t &get_key()
    {
        return key_.current();
    }

    void set_key(const relative_coordinate_t &key)
    {
        key_.set(key);
    }

    unsigned id_;
    piw::controlled_base2_t *client_;
    unsigned state_,savestate_;
    pic::flipflop_t<relative_coordinate_t> key_;
};

struct piw::controlled_base2_t::impl_t
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

struct piw::controller2_t::impl_t: piw::ufilterctl_t
{
    impl_t(piw::controller2_t *h,const piw::cookie_t &c,const std::string &sigmap): filter_(this,piw::cookie_t(0)),sigmask_(0), host_(h), control_manager_(c), light_receiver_(this)
    {
        std::string::const_iterator i,e;
        i=sigmap.begin();
        e=sigmap.end();

        for(; i!=e; ++i)
        {
            unsigned si = (*i)-1;
            sigmask_ |= (1ULL<<(si));
        }
    }

    ~impl_t()
    {
    }

    unsigned long long ufilterctl_thru() { return 0; }
    unsigned long long ufilterctl_outputs() { return 0; }
    unsigned long long ufilterctl_inputs() { return sigmask_; }

    void get_controlled(ctlset_t &ctlset, const piw::data_t &key, const piw::data_t &columnlen)
    {
        ctlset.clear();

        float kc = 0;
        float kr = 0;

        if(!piw::decode_key(key,&kc,&kr)) return;
        if(kc==0 && kr==0) { return; }

        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t lg(controllist_);
        const std::vector<ctlsignal_t *> &l(lg.value());
        std::vector<ctlsignal_t *>::const_iterator li;

        for(li=l.begin();li!=l.end();li++)
        {
            if((*li)==0)
            {
                continue;
            }

            pic::flipflop_t<relative_coordinate_t>::guard_t g((*li)->key_);

            if(g.value().is_valid() && g.value().equals(kc,kr,columnlen))
            {
                ctlset.insert((*li)->client_);
            }

        }

    }

    piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &)
    {
        controller_event_wire_t *c = new controller_event_wire_t(this);
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
                c->set_status(value);
            }
        }
        update_lights();
    }

    void set_key(unsigned index, const relative_coordinate_t &key)
    {
        if(!key.is_valid()) return;

        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t gc(controllist_);

        if(index<gc.value().size())
        {
            ctlsignal_t *c = gc.value()[index];

            if(c)
            {
                c->set_key(key);
            }
        }
        update_lights();
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
        update_lights();
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
        update_lights();
    }

    bool attach_to(controlled_base2_t *c, unsigned n)
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
        l[n]=cs;
        controllist_.exchange();
        update_lights();

        return true;
    }

    unsigned attach(controlled_base2_t *c)
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
        l[i]=cs;
        controllist_.exchange();
        update_lights();

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

    void update_lights()
    {
        pic::flipflop_t<std::vector<ctlsignal_t *> >::guard_t g(controllist_);
        lightset_t lights;

        for(unsigned i=0; i<g.value().size(); ++i)
        {
            ctlsignal_t *c = g.value()[i];

            if(c)
            {
                unsigned cs = c->get_status();
                unsigned cid = c->get_id();
                unsigned fs = light_receiver_.fetch_light(cid,cs);

                if(fs)
                {
                    lights.insert(light_t(c->get_key(),fs));
                }
            }
        }

        control_manager_.update_lights(lights);
    }

    piw::ufilter_t filter_;
    pic::flipflop_t<std::vector<ctlsignal_t *> > controllist_;
    unsigned long sigmask_;
    piw::controller2_t *host_;
    control_manager_t control_manager_;
    light_receiver_t light_receiver_;
};

void controller_event_wire_t::erase_controlled(piw::controlled_base2_t *c)
{
    controlled_.erase(c);
}

void controller_event_wire_t::end__(unsigned long long t)
{
    ctlset_t::const_iterator it;

    for(it=controlled_.begin(); it!=controlled_.end(); ++it)
    {
        (*it)->control_term(t);
    }
}

void controller_event_wire_t::ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long t)
{
    if(!id_.is_null())
    {
        end__(t);
        id_ = piw::data_t();
    }
}

void controller_event_wire_t::start__(unsigned long long t)
{
    controller_->get_controlled(controlled_,current_key_,current_ctl_);

    ctlset_t::const_iterator it;

    for(it=controlled_.begin(); it!=controlled_.end(); ++it)
    {
        (*it)->control_start(t);
    }
}

void controller_event_wire_t::ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
{
    piw::data_nb_t k,c;

    id_ = id.make_normal();

    env->ufilterenv_latest(1,k,id.time());


    current_ctl_ = piw::data_t();
    current_key_ = piw::data_t();

    lref_t layout =  controller_->control_manager_.layout(id_);

    if(layout.isvalid())
        current_ctl_ = layout->columns_definition_;

    env->ufilterenv_start(id.time());

    start__(id.time());
}

void controller_event_wire_t::ufilterfunc_data(piw::ufilterenv_t *env,unsigned s,const piw::data_nb_t &d)
{
    if(!id_.is_null())
    {
        bool changed = false;

        if(s==1)
        {
            if(!compare_phys_key(d,current_key_))
            {
                current_key_ = d.make_normal();
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

piw::controller2_t::controller2_t(const piw::cookie_t &c,const std::string &sigmap): impl_(new impl_t(this,c,sigmap))
{
}

piw::controller2_t::~controller2_t()
{
    delete impl_;
}

piw::controlled_base2_t::controlled_base2_t() : controller_(0), xximpl_(new impl_t())
{
}

piw::controlled_base2_t::~controlled_base2_t()
{
    detach();
    piw::tsd_fastcall(__destruct,this,0);
    delete xximpl_;
}

int piw::controlled_base2_t::__destruct(void *self_, void *)
{
    controlled_base2_t *self = (controlled_base2_t *)self_;
    self->end_event(piw::tsd_time());
    return 1;
} 

bool piw::controlled_base2_t::attach_to(controller2_t *c,unsigned n)
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

void piw::controlled_base2_t::attach(controller2_t *c)
{
    if(c!=controller_)
    {
        detach();
        index_=c->impl_->attach(this);
        controller_=c;
        piw::tsd_fastcall(__init,this,0);
    }
}

int piw::controlled_base2_t::ordinal()
{
    return controller_?(int)index_:-1;
}

void piw::controlled_base2_t::detach()
{
    if(controller_)
    {
        controller_->impl_->set_light(index_,0);
        controller_->impl_->detach(index_);
        controller_=0;
    }
}

void piw::controlled_base2_t::set_self_light(unsigned value)
{
    set_light(ordinal(),value);
}

void piw::controlled_base2_t::set_light(unsigned index, unsigned value)
{
    if(controller_)
        controller_->impl_->set_light(index,value);
}

void piw::controlled_base2_t::save_lights()
{
    if(controller_)
        controller_->impl_->save_lights();
}

void piw::controlled_base2_t::restore_lights()
{
    if(controller_)
        controller_->impl_->restore_lights();
}

piw::fastdata_t *piw::controlled_base2_t::fastdata()
{
    return xximpl_->fastdata();
}

piw::change_nb_t piw::controlled_base2_t::sender()
{
    return xximpl_->sender();
}

void piw::controlled_base2_t::set_sequential_key(int key)
{
    if(controller_)
        controller_->impl_->set_key(index_,key);
}

void piw::controlled_base2_t::set_key(const piw::coordinate_t &key)
{
    if(controller_)
        controller_->impl_->set_key(index_,key);
}

void piw::controlled_base2_t::send(const piw::data_nb_t &d)
{
    xximpl_->send(d);
}

void piw::controlled_base2_t::start_event(unsigned long long t)
{
    xximpl_->start_event(t);
}

void piw::controlled_base2_t::end_event(unsigned long long t)
{
    xximpl_->end_event(t);
}


piw::cookie_t piw::controller2_t::controller_cookie()
{
    return impl_->control_manager_.cookie();
}

piw::cookie_t piw::controller2_t::event_cookie()
{
    return impl_->filter_.cookie();
}

piw::cookie_t piw::controller2_t::converter_cookie()
{
    return impl_->light_receiver_.cookie();
}

static int __start(void *c_, void *)
{
    piw::controlled2_t *c = (piw::controlled2_t *)c_;
    unsigned long long t = piw::tsd_time();
    c->start_event(t);
    return 0;
}

static int __term(void *c_, void *)
{
    piw::controlled2_t *c = (piw::controlled2_t *)c_;
    unsigned long long t = piw::tsd_time();
    c->end_event(t);
    return 0;
}

piw::controlled2_t::controlled2_t(bool enabled): enabled_(enabled)
{
    piw::tsd_fastcall(__start,this,0);
}

piw::controlled2_t::~controlled2_t()
{
    piw::tsd_fastcall(__term,this,0);
}

void piw::controlled2_t::control_init()
{
    set_light(ordinal(),enabled_?1:3);
}

int piw::controlled2_t::__enable_direct(void *c_, void *_)
{
    piw::controlled2_t *c = (piw::controlled2_t *)c_;
    c->control_enable();
    c->control_init();
    return 0;
}

int piw::controlled2_t::__disable_direct(void *c_, void *_)
{
    piw::controlled2_t *c = (piw::controlled2_t *)c_;
    c->control_disable();
    c->control_init();
    return 0;
}

void piw::controlled2_t::control_receive(unsigned s,const piw::data_nb_t &value)
{
    piw::hardness_t h;

    if(s!=1 || !piw::decode_key(value,0,0,0,0,&h) || h==piw::KEY_LIGHT) return;

    if(enabled_)
        disable();
    else
        enable();
}

void piw::controlled2_t::enable()
{
    if(!enabled_)
    {
        enabled_=true;
        tsd_fastcall(__enable_direct,this,0);
    }
}

void piw::controlled2_t::disable()
{
    if(enabled_)
    {
        enabled_=false;
        tsd_fastcall(__disable_direct,this,0);
    }
}

piw::ufilterfunc_t *control_manager_t::ufilterctl_create(const piw::data_t &)
{
    return new control_manager_wire_t(this);
}

void control_manager_t::ufilterctl_delete(piw::ufilterfunc_t *f)
{
    if(f)
    {
        delete f;
    }
}

void control_manager_t::control_changed(const piw::data_t &id)
{
    pic::lcklist_t<control_manager_subscriber_t *>::nbtype::iterator i;

    for(i=subscribers_.begin(); i!=subscribers_.end(); i++)
    {
        (*i)->control_changed(id);
    }
}


lref_t control_manager_t::layout(const piw::data_t &id)
{
    pic::lckmap_t<piw::data_t,lref_t,piw::path_less>::nbtype::iterator i;

    const unsigned char *p = id.as_path();
    int l = id.as_pathlen();

    for(;l>=0;l--)
    {
        piw::data_t d  = piw::makepath(p,l);
        pic::lckmap_t<piw::data_t,lref_t,piw::path_less>::nbtype::iterator li;

        li=layout_.find(d);

        if(li!=layout_.end())
        {
            return li->second;
        }
    }

    return lref_t();
}

layout_t::layout_t(const piw::data_t &offsets, const piw::data_t &lengths, const piw::data_t &columns): offsets_definition_(offsets), lengths_definition_(lengths), columns_definition_(columns)
{
}

void light_receiver_wire_t::ufilterfunc_changed(piw::ufilterenv_t *, void *)
{
}

void light_receiver_wire_t::ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
{
    if(id.as_pathlen()==0)
    {
        return;
    }

    channel_ = id.as_path()[0];
    piw::data_nb_t d;

    if(e->ufilterenv_latest(1,d,id.time()))
    {
        color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
    }

    impl_->add_input(this,channel_);
}

void light_receiver_wire_t::ufilterfunc_data(piw::ufilterenv_t *,unsigned s,const piw::data_nb_t &d)
{
    if(channel_>0 && s==1)
    {
        color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
        impl_->update_input(channel_);
    }
}

void light_receiver_wire_t::ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
{
    if(channel_>0)
    {
        impl_->del_input(this,channel_);
        channel_=0;
    }
}

void light_receiver_t::receiver_changed()
{
    impl_->update_lights();
}

