
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

#include <piw/piw_tsd.h>
#include <piw/piw_lights.h>
#include <piw/piw_status.h>
#include <piw/piw_thing.h>
#include <piw/piw_ufilter.h>
#include <picross/pic_ref.h>
#include <picross/pic_ilist.h>

namespace
{
    struct receiver_t: piw::ufilterfunc_t, virtual pic::lckobject_t, pic::element_t<0>
    {
        receiver_t(piw::lightconvertor_t::impl_t *root): impl_(root), color_(0)
        {
        }

        ~receiver_t()
        {
        }

        void ufilterfunc_changed(piw::ufilterenv_t *, void *);
        void ufilterfunc_start(piw::ufilterenv_t *, const piw::data_nb_t &);
        void ufilterfunc_data(piw::ufilterenv_t *, unsigned,const piw::data_nb_t &);
        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long);

        piw::lightconvertor_t::impl_t *impl_;
        unsigned color_;
        piw::coordinate_t light_;
    };

    struct list_t: virtual pic::lckobject_t, virtual pic::atomic_counted_t, pic::nocopy_t
    {
        pic::ilist_t<receiver_t> list_;
    };

    struct handler_t
    {
        handler_t(const piw::coordinate_t &key, const piw::change_t &h): key_(key), handler_(h), default_(3)
        {
        }

        piw::coordinate_t key_;
        piw::change_t handler_;
        unsigned default_;
    };

    typedef pic::ref_t<list_t> listref_t;
};

struct piw::lightconvertor_t::impl_t: virtual pic::lckobject_t, piw::ufilterctl_t, piw::ufilter_t, virtual pic::tracked_t
{
    impl_t(bool musical, const piw::cookie_t &output): piw::ufilter_t(this,piw::cookie_t(0)), musical_(musical), output_(piw::change_nb_t(),0,output), size_(0)
    {
    }

    ~impl_t()
    {
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &path)
    {
        return new receiver_t(this);
    }

    void update_input(const piw::coordinate_t &light)
    {
        pic::lckmap_t<piw::coordinate_t,listref_t>::lcktype::iterator i;
        pic::flipflop_t<std::map<unsigned,handler_t> >::guard_t g(status_handlers_);
        std::map<unsigned,handler_t>::const_iterator it;

        bool ds = false;
        unsigned d = 3;

        for(it=g.value().begin(); it!=g.value().end(); it++)
        {
            if(it->second.key_==light)
            {
                if(!ds)
                {
                    d = it->second.default_;
                    ds = true;
                }
                else
                {
                    if(d!=it->second.default_)
                    {
                        d=3;
                    }
                }
            }
        }

        i = inputs_.find(light);

        if(i==inputs_.end())
        {
            output_.set_status(musical_,light,0);
            return;
        }

        receiver_t *receiver;
        list_t *l = i->second.ptr();

        unsigned m = CLR_ORANGE;
        unsigned color;
        bool f = false;

        for(receiver=l->list_.head(); receiver!=0; receiver=l->list_.next(receiver))
        {
            color = receiver->color_;
            f = true;

            if(color==CLR_ORANGE)
            {
                continue;
            }

            if(m==CLR_ORANGE)
            {
                m=color;
                continue;
            }

            if(color==CLR_MIXED)
            {
                m=color;
                continue;
            }

            if(m!=color)
            {
                m=CLR_MIXED;
            }
        }

        if(!f)
        {
            m=CLR_OFF;
        }

        switch(m)
        {
            case CLR_MIXED: color=3; break;
            case CLR_OFF: color=0; break;
            case CLR_RED: color=2; break;
            case CLR_GREEN: color=1; break;
            default: color=d; break;
        }


        unsigned status=BCTSTATUS_UNKNOWN;
        switch(color)
        {
            case 0: status=BCTSTATUS_OFF; break;
            case 1: status=BCTSTATUS_ACTIVE; break;
            case 2: status=BCTSTATUS_INACTIVE; break;
            case 3: status=BCTSTATUS_UNKNOWN; break;
        }

        output_.set_status(musical_,light,status);

        for(it=g.value().begin(); it!=g.value().end(); it++)
        {
            if(it->second.key_ == light)
            {
                it->second.handler_(makelong(status,piw::tsd_time()));
            }
        }
    }

    void add_input(receiver_t *receiver, const piw::coordinate_t &light)
    {
        std::pair<pic::lckmap_t<piw::coordinate_t,listref_t>::lcktype::iterator,bool> i;
        i=inputs_.insert(std::make_pair(light,listref_t()));

        if(i.second)
        {
            i.first->second = pic::ref(new list_t);
        }

        i.first->second->list_.append(receiver);
        update_input(light);
    }

    void del_input(receiver_t *receiver, const piw::coordinate_t &light)
    {
        pic::lckmap_t<piw::coordinate_t,listref_t>::lcktype::iterator i;
        i = inputs_.find(light);

        if(i!=inputs_.end())
        {
            i->second->list_.remove(receiver);
            update_input(i->first);
        }

    }

    static int update_input__(void *s_, void *l_)
    {
        impl_t *self = (impl_t *)s_;
        piw::coordinate_t light = *(piw::coordinate_t *)l_;
        self->update_input(light);
        return 0;
    }

    void set_default_color(unsigned i, unsigned color)
    {
        std::map<unsigned,handler_t>::iterator it;

        if((it=status_handlers_.alternate().find(i)) != status_handlers_.alternate().end())
        {
            it->second.default_ = color;
            status_handlers_.exchange();
            piw::coordinate_t light(it->second.key_);
            piw::tsd_fastcall(update_input__,this,(void *)&light);
        }
    }

    void set_status_handler(unsigned index, const piw::coordinate_t &key, change_t f)
    {
        status_handlers_.alternate().insert(std::make_pair(index,handler_t(key,f)));
        status_handlers_.exchange();
    }

    void remove_status_handler(unsigned index)
    {
        status_handlers_.alternate().erase(index);
        status_handlers_.exchange();
    }

    unsigned char get_status(const piw::coordinate_t &key)
    {
        return output_.get_status(musical_,key);
    }

    unsigned long long ufilterctl_thru() { return 0; }
    unsigned long long ufilterctl_inputs() { return SIG2(1,2); }
    unsigned long long ufilterctl_outputs() { return 0; }

    const bool musical_;
    piw::statusbuffer_t output_;
    pic::lckmap_t<piw::coordinate_t,listref_t>::lcktype inputs_;
    unsigned size_;
    pic::flipflop_t<std::map<unsigned,handler_t> > status_handlers_;
};

piw::lightconvertor_t::lightconvertor_t(bool musical, const piw::cookie_t &output): impl_(new impl_t(musical, output))
{
}

piw::lightconvertor_t::~lightconvertor_t()
{
    delete impl_;
}

void piw::lightconvertor_t::set_status_handler(unsigned index, const piw::coordinate_t &key, change_t f)
{
    impl_->set_status_handler(index,key,f);
}

void piw::lightconvertor_t::remove_status_handler(unsigned index)
{
    impl_->remove_status_handler(index);
}

unsigned char piw::lightconvertor_t::get_status(const piw::coordinate_t &key)
{
    return impl_->get_status(key);
}

piw::cookie_t piw::lightconvertor_t::cookie()
{
    return impl_->cookie();
}

void receiver_t::ufilterfunc_changed(piw::ufilterenv_t *, void *)
{
}

void piw::lightconvertor_t::set_default_color(unsigned l,unsigned color)
{
    impl_->set_default_color(l,color);
}

void receiver_t::ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
{
    piw::data_nb_t d;

    if(e->ufilterenv_latest(1,d,id.time()))
    {
        color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
    }

    light_ = piw::coordinate_t();

    if(e->ufilterenv_latest(2,d,id.time()))
    {
        light_ = piw::coordinate_t(d);
        if(light_.is_valid())
        {
            impl_->add_input(this,light_);
        }
    }
}

void receiver_t::ufilterfunc_data(piw::ufilterenv_t *, unsigned s, const piw::data_nb_t &d)
{
    if(s==1)
    {
        color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
        if(light_.is_valid())
        {
            impl_->update_input(light_);
        }
    }

    if(s==2)
    {
        piw::coordinate_t light(d);
        if(light.is_valid() && (light != light_))
        {
            impl_->del_input(this,light_);
            light_ = light;
            impl_->add_input(this,light_);
        }
    }
}

void receiver_t::ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
{
    if(light_.is_valid())
    {
        impl_->del_input(this,light_);
    }
}
