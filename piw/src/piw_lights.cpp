
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
#include <piw/piw_thing.h>
#include <picross/pic_ref.h>

namespace
{
    struct light_t: piw::wire_ctl_t, piw::event_data_source_real_t, pic::counted_t, virtual pic::lckobject_t
    {
        light_t(unsigned index): piw::event_data_source_real_t(piw::pathone(index,0)), id_(index), color_(0), colorsave_(-1)
        {
        }

        ~light_t()
        {
            source_shutdown();
        }

        void save(unsigned color)
        {
            colorsave_=color_;
            set_color(color);
        }

        void restore()
        {
            if(colorsave_>=0)
            {
                set_color(colorsave_);
                colorsave_=-1;
            }
        }

        void set_color2(unsigned color)
        {
            if(colorsave_>=0)
            {
                colorsave_=color;
            }
            else
            {
                set_color(color);
            }
        }

        static int __setcolor(void *l_, void *c_)
        {
            light_t *l = (light_t *)l_;
            unsigned color = *(unsigned *)c_;

            if(color==l->color_)
            {
                return 0;
            }

            if(color)
            {
                if(l->color_)
                {
                    l->buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,color,piw::tsd_time()));
                }
                else
                {
                    l->buffer_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
                    unsigned long long t = piw::tsd_time();
                    l->buffer_.add_value(1,piw::makefloat_bounded_nb(5.0,0.0,0.0,color,t));
                    l->source_start(0,piw::pathone_nb(l->id_,t),l->buffer_);
                }
            }
            else
            {
                if(l->color_)
                {
                    l->source_end(piw::tsd_time());
                }
            }

            l->color_=color;
            return 0;
        }

        void set_color(unsigned color)
        {
            piw::tsd_fastcall(__setcolor,this,&color);
        }

        unsigned id_;
        unsigned color_;
        int colorsave_;
        piw::xevent_data_buffer_t buffer_;
    };

    typedef pic::ref_t<light_t> lightref_t;
    typedef pic::lckvector_t<lightref_t>::lcktype lightvector_t;
};

struct piw::lightsource_t::impl_t: piw::root_ctl_t, piw::thing_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(const piw::change_nb_t &s, unsigned ch, const piw::cookie_t &c): lswitch_(s), blinking_(false), override_(false), channel_(ch)
    {
        piw::tsd_thing(this);
        connect(c);
    }

    ~impl_t()
    {
        tracked_invalidate();
    }

    void set_size(unsigned lights)
    {
        lightvector_t &v(lights_.alternate());

        unsigned osize = v.size();

        if(osize==lights)
        {
            return;
        }

        if(osize>lights)
        {
            v.resize(lights);
            lights_.exchange();
            return;
        }

        v.resize(lights);

        while(osize<lights)
        {
            pic::ref_t<light_t> l = pic::ref(new light_t(osize+1));
            connect_wire(l.ptr(),l->source());
            v[osize]=l;
            osize++;
        }

        lights_.exchange();
    }

    void light(const piw::data_nb_t &d)
    {
        current_.set_nb(d);

        if(!blinking_ && !override_)
        {
            lswitch_(d);
        }
    }

    static int __enabler(void *r_, void *i_)
    {
        impl_t *self = (impl_t *)r_;
        unsigned input = *(unsigned *)i_;
        self->light(piw::makelong_nb(input,piw::tsd_time()));
        return 0;
    }

    void enable(unsigned d)
    {
        piw::tsd_fastcall(__enabler,this,&d);
    }

    void blink(const piw::data_nb_t &d)
    {
        if(d.is_null() || d.as_norm()==0 || blinking_)
        {
            return;
        }

        pic::logmsg() << "blink..";

        blinking_=true;

        if(!override_)
        {
            lswitch_(piw::makelong_nb(channel_,piw::tsd_time()));
        }

        enqueue_fast(piw::makelong_nb(0,0));
    }

    void thing_dequeue_fast(const piw::data_nb_t &d)
    {
        unsigned i = d.as_long();
        pic::flipflop_t<lightvector_t>::guard_t g(lights_);
        if(i<g.value().size())
        {
            lightref_t l(g.value()[i]);
            l->save(3);
            enqueue_fast(piw::makelong_nb(i+1,0));
        }
        else
        {
            timer_fast(500);
        }
    }

    static int overrider__(void *self_, void *override_)
    {
        bool override = *(bool *)override_;
        impl_t *self = (impl_t *)self_;

        if(override==self->override_)
        {
            return 1;
        }

        self->override_=override;

        if(!self->blinking_)
        {
            if(override)
            {
                self->lswitch_(piw::makelong_nb(self->channel_,piw::tsd_time()));
            }
            else
            {
                self->lswitch_(self->current_.get().restamp(piw::tsd_time()));
            }
        }

        return 1;
    }

    static int setter__(void *light_, void *color_)
    {
        light_t *light = (light_t *)light_;
        unsigned color = *(unsigned *)color_;
        light->set_color2(color);
        return 1;
    }

    void override(bool override)
    {
        piw::tsd_fastcall(overrider__,this,&override);
    }

    void set_color(unsigned light,unsigned color)
    {
        //pic::logmsg() << "set_color: light=" << light << " color=" << color;
        lightvector_t &v(lights_.alternate());
        
        if(light<=0 || light>v.size())
        {
            return;
        }

        piw::tsd_fastcall(setter__,v[light-1].ptr(),&color);
    }

    void thing_timer_fast()
    {
        if(!blinking_)
        {
            return;
        }

        blinking_=false;
        cancel_timer_fast();

        if(!override_)
        {
            lswitch_(current_.get().restamp(piw::tsd_time()));
        }

        pic::flipflop_t<lightvector_t>::guard_t g(lights_);

        for(unsigned i=0;i<g.value().size();i++)
        {
            lightref_t l(g.value()[i]);
            l->restore();
        }
    }

    pic::flipflop_t<lightvector_t> lights_;
    pic::flipflop_functor_t<piw::change_nb_t> lswitch_;
    bool blinking_,override_;
    piw::dataholder_nb_t current_;
    unsigned channel_;
};

piw::change_nb_t piw::lightsource_t::lightswitch()
{
    return piw::change_nb_t::method(root_,&impl_t::light);
}

piw::change_nb_t piw::lightsource_t::blinker()
{
    return piw::change_nb_t::method(root_,&impl_t::blink);
}

piw::lightsource_t::lightsource_t(const piw::change_nb_t &s, unsigned ch, const cookie_t &c): root_(new impl_t(s,ch,c))
{
}

piw::lightsource_t::~lightsource_t()
{
    delete root_;
}

void piw::lightsource_t::override(bool over)
{
    root_->override(over);
}

void piw::lightsource_t::set_color(unsigned light, unsigned color)
{
    root_->set_color(light,color);
}

void piw::lightsource_t::set_status(unsigned light, unsigned status)
{
    unsigned c=0;
    switch(status)
    {
        case BCTSTATUS_MIXED: c=3; break;
        case BCTSTATUS_OFF: c=0; break;
        case BCTSTATUS_ACTIVE: c=1; break;
        case BCTSTATUS_INACTIVE: c=2; break;
    }
    set_color(light,c);
}

void piw::lightsource_t::set_size(unsigned lights)
{
    root_->set_size(lights);
}

unsigned piw::lightsource_t::get_size()
{
    return root_->lights_.alternate().size();
}

int piw::lightsource_t::gc_traverse(void *v, void *a) const
{
    return root_->lswitch_.gc_traverse(v,a);
}

int piw::lightsource_t::gc_clear()
{
    return root_->lswitch_.gc_clear();
}

void piw::lightsource_t::enable(unsigned input)
{
    root_->enable(input);
}
