
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
        receiver_t(piw::lightconvertor_t::impl_t *root): impl_(root), color_(0), active_(false)
        {
        }

        ~receiver_t()
        {
        }

        void ufilterfunc_changed(piw::ufilterenv_t *, void *);
        void ufilterfunc_start(piw::ufilterenv_t *,const piw::data_nb_t &);
        void ufilterfunc_data(piw::ufilterenv_t *,unsigned,const piw::data_nb_t &);
        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long);

        piw::lightconvertor_t::impl_t *impl_;
        unsigned color_;
        unsigned light_;
        bool active_;
    };

    struct list_t: virtual pic::lckobject_t, virtual pic::atomic_counted_t, pic::nocopy_t
    {
        list_t(): default_(3)
        {
        }

        pic::ilist_t<receiver_t> list_;
        unsigned default_;
    };

    typedef pic::ref_t<list_t> listref_t;
};

struct piw::lightconvertor_t::impl_t: virtual pic::lckobject_t, piw::ufilterctl_t, piw::ufilter_t, virtual pic::tracked_t
{
    impl_t(const piw::cookie_t &output): piw::ufilter_t(this,piw::cookie_t(0)), output_(piw::change_nb_t(),0,output), size_(0)
    {
    }

    ~impl_t()
    {
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &path)
    {
        return new receiver_t(this);
    }

    void update_input(const unsigned i)
    {
        if(i==0)
        {
            return;
        }

        ensure_size(i);

        receiver_t *r;
        list_t *l = inputs_[i-1].ptr();

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
            m=CLR_OFF;
        }

        switch(m)
        {
            case CLR_MIXED: c=3; break;
            case CLR_OFF: c=0; break;
            case CLR_RED: c=2; break;
            case CLR_GREEN: c=1; break;
            default: c=l->default_; break;
        }


        unsigned status=BCTSTATUS_UNKNOWN;
        switch(c)
        {
            case 0: status=BCTSTATUS_OFF; break;
            case 1: status=BCTSTATUS_ACTIVE; break;
            case 2: status=BCTSTATUS_INACTIVE; break;
            case 3: status=BCTSTATUS_UNKNOWN; break;
        }

        output_.set_status(i,status);

        pic::flipflop_t<std::map<unsigned,change_t> >::guard_t g(status_handlers_);
        std::map<unsigned,change_t>::const_iterator it = g.value().find(i);
        if (it != g.value().end())
        {
            it->second(makelong(status,piw::tsd_time()));
        }
    }

    void add_input(receiver_t *r, unsigned i)
    {
        if(i>0)
        {
            ensure_size(i);
            inputs_[i-1]->list_.append(r);
            update_input(i);
        }
    }

    void del_input(receiver_t *r, unsigned i)
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
                inputs_[i] = pic::ref(new list_t);
            }
        }

        size_ = new_size;
    }

    static int default_color__(void *s_, void *i_, void *c_)
    {
        impl_t *self = (impl_t *)s_;
        unsigned c = *(unsigned *)c_;
        unsigned i = *(unsigned *)i_;

        if(i==0)
        {
            return 0;
        }

        self->ensure_size(i);

        list_t *l = self->inputs_[i-1].ptr();

        if(l->default_ != c)
        {
            l->default_=c;
            self->update_input(i);
        }

        return 0;
    }

    void set_default_color(unsigned i, unsigned c)
    {
        piw::tsd_fastcall3(default_color__,this,&i,&c);
    }

    void set_status_handler(unsigned index, change_t f)
    {
        status_handlers_.alternate().insert(std::pair<unsigned,change_t>(index,f));
        status_handlers_.exchange();
    }

    void remove_status_handler(unsigned index)
    {
        status_handlers_.alternate().erase(index);
        status_handlers_.exchange();
    }

    unsigned char get_status(unsigned index)
    {
        return output_.get_status(index);
    }

    unsigned long long ufilterctl_thru() { return 0; }
    unsigned long long ufilterctl_inputs() { return SIG1(1); }
    unsigned long long ufilterctl_outputs() { return 0; }

    piw::statusbuffer_t output_;
    pic::lckvector_t<listref_t>::lcktype inputs_;
    unsigned size_;
    pic::flipflop_t<std::map<unsigned,change_t> > status_handlers_;
};

piw::lightconvertor_t::lightconvertor_t(const piw::cookie_t &output): impl_(new impl_t(output))
{
}

piw::lightconvertor_t::~lightconvertor_t()
{
    delete impl_;
}

void piw::lightconvertor_t::set_status_handler(unsigned index, change_t f)
{
    impl_->set_status_handler(index, f);
}

void piw::lightconvertor_t::remove_status_handler(unsigned index)
{
    impl_->remove_status_handler(index);
}

unsigned char piw::lightconvertor_t::get_status(unsigned index)
{
    return impl_->get_status(index);
}

piw::cookie_t piw::lightconvertor_t::cookie()
{
    return impl_->cookie();
}

void receiver_t::ufilterfunc_changed(piw::ufilterenv_t *, void *)
{
}

void piw::lightconvertor_t::set_default_color(unsigned l,unsigned c)
{
    impl_->set_default_color(l,c);
}

void receiver_t::ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
{
    unsigned l = id.as_pathgristlen();

    if(l>0)
    {
        light_ = id.as_pathgrist()[0];
        piw::data_nb_t d;

        if(e->ufilterenv_latest(1,d,id.time()))
        {
            color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
        }

        active_ = true;
        impl_->add_input(this,light_);
    }
}

void receiver_t::ufilterfunc_data(piw::ufilterenv_t *,unsigned s,const piw::data_nb_t &d)
{
    if(active_ && s==1)
    {
        color_ = (unsigned)d.as_renorm(0.0,5.0,0.0);
        impl_->update_input(light_);
    }
}

void receiver_t::ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
{
    if(active_)
    {
        impl_->del_input(this,light_);
        active_=false;
    }
}
