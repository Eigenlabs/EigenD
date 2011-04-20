
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
#include <piw/piw_controllerdict.h>
#include <piw/piw_thing.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_ref.h>

struct piw::controllerdict_t::impl_t: piw::event_data_source_real_t, piw::thing_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(const piw::cookie_t &c): piw::event_data_source_real_t(piw::pathnull(piw::tsd_time()))
    {
        piw::tsd_thing(this);
        root_.connect(c);
        root_.connect_wire(&wire_, source());
        buffer_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        piw::tsd_fastcall(init__,this,0);
    }

    ~impl_t()
    {
        source_end(piw::tsd_time());
        source_shutdown();
        tracked_invalidate();
    }

    static int init__(void *self_, void *arg_)
    {
        impl_t *self = (impl_t *)self_;

        unsigned long long t = piw::tsd_time();
        self->buffer_.add_value(1,dictnull_nb(t));
        self->source_start(0,piw::pathnull_nb(t),self->buffer_);
        
        return 1;
    }

    piw::data_t get_ctl_value(const std::string &k)
    {
        bct_data_t d(0);
        piw::tsd_fastcall3(get__,(void *)&ctl_dict_,(void *)&k,(void *)&d);
        return piw::data_t::from_given(d);
    }

    static int get__(void *m_, void *k_, void *v_)
    {
        std::map<std::string,piw::data_nb_t> *m = (std::map<std::string,piw::data_nb_t> *)m_;
        const std::string k = *(const std::string *)k_;
        bct_data_t *v = (bct_data_t *)v_;

        std::map<std::string,piw::data_nb_t>::iterator it;
        it = m->find(k);
        if(it!=m->end())
        {
            *v=it->second.give_copy();
        }
        else
        {
            *v=makenull(0).give_copy();
        }
        
        return 1;
    }

    piw::data_t get_ctl_dict()
    {
        bct_data_t d(0);
        piw::tsd_fastcall3(get_dict__,(void *)&ctl_dict_,(void *)&d,0);
        return piw::data_t::from_given(d);
    }

    static int get_dict__(void *m_, void *v_, void *)
    {
        std::map<std::string,piw::data_nb_t> *m = (std::map<std::string,piw::data_nb_t> *)m_;
        bct_data_t *v = (bct_data_t *)v_;
            
        unsigned long long t = piw::tsd_time();
        piw::data_nb_t dict = dictnull_nb(t);

        std::map<std::string,piw::data_nb_t>::iterator it;
        for(it=m->begin(); it!=m->end(); it++)
        {
            dict=dictset_nb(dict, it->first, it->second);
        }

        *v=dict.give_copy();

        return 1;
    }

    void put_ctl(const std::string &k,const piw::data_t &v)
    {
        piw::tsd_fastcall4(put__,this,(void *)&ctl_dict_,(void *)&k,(void *)v.give_copy(PIC_ALLOC_NB));
    }

    void put(const std::string &k,const piw::data_t &v)
    {
        piw::tsd_fastcall4(put__,this,(void *)&dict_,(void *)&k,(void *)v.give_copy(PIC_ALLOC_NB));
    }

    static int put__(void *self_, void *m_, void *k_, void *v_)
    {
        impl_t *self = (impl_t *)self_;
        std::map<std::string,piw::data_nb_t> *m = (std::map<std::string,piw::data_nb_t> *)m_;
        const std::string k = *(const std::string *)k_;
        const piw::data_nb_t v = piw::data_nb_t::from_given((bct_data_t)v_);

        if(v.is_null() || (v.is_string() && 0==v.as_stringlen()))
        {
            m->erase(k);
        }
        else
        {
            (*m)[k]=v;
        }

        self->send();
        
        return 1;
    }

    void send()
    {
        piw::data_nb_t dict = dictnull_nb(piw::tsd_time());

        pic::lckmap_t<std::string,piw::data_nb_t>::nbtype::iterator it;
        for(it=ctl_dict_.begin(); it!=ctl_dict_.end(); it++)
        {
            dict = dictset_nb(dict, it->first, it->second);
        }
        for(it=dict_.begin(); it!=dict_.end(); it++)
        {
            dict = dictset_nb(dict, it->first, it->second);
        }

        buffer_.add_value(1,dict);
    }

    piw::root_ctl_t root_;
    piw::wire_ctl_t wire_;
    piw::xevent_data_buffer_t buffer_;
    pic::lckmap_t<std::string,piw::data_nb_t>::nbtype ctl_dict_;
    pic::lckmap_t<std::string,piw::data_nb_t>::nbtype dict_;
};

piw::controllerdict_t::controllerdict_t(const cookie_t &c): root_(new impl_t(c))
{
}

piw::controllerdict_t::~controllerdict_t()
{
}

piw::data_t piw::controllerdict_t::get_ctl_value(const std::string &k)
{
    return root_->get_ctl_value(k);
}

piw::data_t piw::controllerdict_t::get_ctl_dict()
{
    return root_->get_ctl_dict();
}

void piw::controllerdict_t::put_ctl(const std::string &k,const piw::data_t &v)
{
    root_->put_ctl(k,v);
}

void piw::controllerdict_t::put(const std::string &k,const piw::data_t &v)
{
    root_->put(k,v);
}

namespace piw
{
    struct changetonicsink_t: pic::sink_t<void(const piw::data_t &)>
    {
        changetonicsink_t(controllerdict_t::impl_t *i, unsigned t) : impl_(i), tonic_(t), tonic_data_(makefloat_bounded(12, 0, 0, t, 0L))
        {
        }

        void invoke(const piw::data_t &d) const
        {
            if(d.as_norm()!=0)
            {
                pic::weak_t<controllerdict_t::impl_t>::guard_t g(impl_);
                g.value()->put("tonic", tonic_data_);
            }
        }

        bool iscallable() const
        {
            return impl_.isvalid();
        }

        bool compare(const pic::sink_t<void(const piw::data_t &)> *o) const
        {
            const changetonicsink_t *c = dynamic_cast<const changetonicsink_t *>(o);
            return c ? c->impl_==impl_ && c->tonic_==tonic_ : false;
        }

        pic::weak_t<controllerdict_t::impl_t> impl_;
        unsigned tonic_;
        piw::data_t tonic_data_;
    };

    struct changescalesink_t: pic::sink_t<void(const piw::data_t &)>
    {
        changescalesink_t(controllerdict_t::impl_t *i, const std::string &s) : impl_(i), scale_(s), scale_data_(makestring(s, 0L))
        {
        }

        void invoke(const piw::data_t &d) const
        {
            if(d.as_norm()!=0)
            {
                pic::weak_t<controllerdict_t::impl_t>::guard_t g(impl_);
                g.value()->put("scale", scale_data_);
            }
        }

        bool iscallable() const
        {
            return impl_.isvalid();
        }

        bool compare(const pic::sink_t<void(const piw::data_t &)> *o) const
        {
            const changescalesink_t *c = dynamic_cast<const changescalesink_t *>(o);
            return c ? c->impl_==impl_ && 0 == c->scale_.compare(scale_) : false;
        }

        pic::weak_t<controllerdict_t::impl_t> impl_;
        const std::string scale_;
        piw::data_t scale_data_;
    };
}

piw::change_t piw::controllerdict_t::changetonic(unsigned t) { return piw::change_t(pic::ref(new changetonicsink_t(root_,t))); }
piw::change_t piw::controllerdict_t::changescale(const std::string &s) { return piw::change_t(pic::ref(new changescalesink_t(root_,s))); }
