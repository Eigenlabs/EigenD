
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

#include <piw/piw_slowchange.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_data.h>
#include <picross/pic_log.h>

namespace
{
    struct slowchange_t: piw::thing_t, piw::change_t::sinktype_t, virtual public pic::lckobject_t
    {
        slowchange_t(const piw::change_t &c)
        {
            tsd_thing(this);
            set_slow_dequeue_handler(c);
        }

        int gc_visit(void *v, void *a) const
        {
            return gc_traverse(v,a);
        }

        ~slowchange_t()
        {
            tracked_invalidate();
        }

        void invoke(const piw::data_t &d) const
        {
            slowchange_t *s = const_cast<slowchange_t *>(this);
            s->enqueue_slow(d);
        }

        bool iscallable() const { return true; }
    };

    struct slowtrigger_t: piw::thing_t, piw::change_t::sinktype_t, virtual public pic::lckobject_t
    {
        slowtrigger_t(const piw::change_t &c): active_(false), init_(false)
        {
            tsd_thing(this);
            set_slow_dequeue_handler(c);
        }

        ~slowtrigger_t()
        {
            tracked_invalidate();
        }

        int gc_visit(void *v, void *a) const
        {
            return gc_traverse(v,a);
        }

        void invoke(const piw::data_t &d) const
        {
            slowtrigger_t *s = const_cast<slowtrigger_t *>(this);
            unsigned l = d.as_arraylen();
            const float *f = d.as_array();

            if(!s->init_)
            {
                s->init_=true;
                s->active_=false;

                if(l>0 && d.as_norm()!=0)
                {
                    s->active_=true;
                }

                if(s->active_)
                {
                    s->enqueue_slow_nb(piw::makebool_nb(true,0));
                }
                else
                {
                    s->enqueue_slow_nb(piw::makebool_nb(false,0));
                }

                return;
            }

            while(l>0)
            {
                if(s->active_)
                {
                    while(l>0 && *f!=0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        s->active_=false;
                        f++; l--;

                        try
                        {
                            s->enqueue_slow_nb(piw::makebool_nb(false,0));
                        }
                        CATCHLOG()
                    }
                }
                else
                {
                    while(l>0 && *f==0)
                    {
                        f++; l--;
                    }
                    if(l>0)
                    {
                        s->active_=true;
                        f++; l--;
                        
                        try
                        {
                            s->enqueue_slow_nb(piw::makebool_nb(true,0));
                        }
                        CATCHLOG()
                    }
                }
            }
        }

        bool iscallable() const { return true; }

        mutable bool active_;
        mutable bool init_;
    };

    struct fastchange_t: piw::change_t::sinktype_t
    {
        fastchange_t(const piw::change_nb_t &c): change_(c) {}

        bool iscallable() const { return true; }

        static int __change(void *a1, void *a2)
        {
            piw::change_nb_t *c = (piw::change_nb_t *)a1;
            piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)a2);
            (*c)(d);
            return 0;
        }

        void invoke(const piw::data_t &x) const
        {
            piw::tsd_fastcall(__change, (void *)&change_, (void *)x.give_copy(PIC_ALLOC_NB));
        }

        int gc_visit(void *v, void *a) const
        {
            return change_.gc_traverse(v,a);
        }

        piw::change_nb_t change_;
    };

    struct deferred_sender_t: piw::change_nb_t::sinktype_t
    {
        deferred_sender_t(const piw::change_nb_t &c, const piw::data_nb_t &v): change_(c), value_(v)
        {
        }

        bool iscallable() const { return true; }

        void invoke(const piw::data_nb_t &x) const
        {
            if(!x.is_null() && x.as_norm()!=0)
            {
                pic::logmsg() << "sending deferred " << value_;
                change_(value_.restamp(x.time()));
            }
        }

        int gc_visit(void *v, void *a) const
        {
            return change_.gc_traverse(v,a);
        }

        pic::flipflop_functor_t<piw::change_nb_t> change_;
        piw::data_nb_t value_;
    };

    struct slowpoll_t : piw::change_nb_t::sinktype_t, piw::thing_t
    {
        slowpoll_t(const piw::change_t &slow, unsigned ms) : slow_(slow)
        {
            piw::tsd_thing(this);
            timer_slow(ms);
        }

        void invoke(const piw::data_nb_t &d) const
        {
            value_.set(d);
        }

        bool iscallable() const
        {
            return true;
        }

        void thing_timer_slow()
        {
            piw::data_t d(value_.get());
            if(d!=last_)
            {
                last_ = d;
                slow_(d);
            }
        }

        int gc_visit(void *v, void *a) const
        {
            return slow_.gc_traverse(v,a);
        }

        piw::change_t slow_;
        piw::data_t last_;
        mutable piw::datadrop_t value_;
    };
}

piw::change_t piw::slowchange(const piw::change_t &c)
{
    return piw::change_t(pic::ref(new slowchange_t(c)));
}

piw::change_nb_t piw::slowchange_polled(const piw::change_t &c,unsigned ms)
{
    return piw::change_nb_t(pic::ref(new slowpoll_t(c,ms)));
}

piw::change_t piw::slowtrigger(const piw::change_t &c)
{
    return piw::change_t(pic::ref(new slowtrigger_t(c)));
}

piw::change_t piw::fastchange(const piw::change_nb_t &c)
{
    return piw::change_t(pic::ref(new fastchange_t(c)));
}

piw::change_nb_t piw::deferred_sender(const piw::change_nb_t &c, const piw::data_t &v)
{
    return piw::change_nb_t(pic::ref(new deferred_sender_t(c,v.make_nb())));
}
