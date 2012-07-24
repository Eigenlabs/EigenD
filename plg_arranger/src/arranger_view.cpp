
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

#include "arranger_view.h"
#include "arranger_colrow.h"
#include <picross/pic_stl.h>
#include <picross/pic_log.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_keys.h>
#include <memory>
#include <algorithm>
#include <piarranger_exports.h>

#define MODES 6  // mode 0==normal view

#define THRESHOLD_ONE 0.3f
#define THRESHOLD_MANY 0.5f
#define THRESHOLD_OFF 0.1f
#define FACTOR 0.05f

#define KEY_SCROLL_RIGHT 1
#define KEY_SCROLL_LEFT 2
#define KEY_SCROLL_DOWN 3
#define KEY_SCROLL_UP 4
#define KEY_ARRANGER_ONOFF 5
#define KEY_EVENT_MODE 6
#define KEY_LOOPDURATION_MODE 7
#define KEY_RESETWINDOW 8
#define KEY_CLEARALL 10

#define SIG_PRESSURE 1
#define SIG_ROLL 2
#define SIG_YAW 3
#define SIG_KEY 5

#define DEFAULT_DOUBLETAP 0.5f

namespace
{
    struct PIARRANGER_DECLSPEC_CLASS cell_t: virtual pic::lckobject_t
    {
        cell_t() { clear(); }

        void set(unsigned z,unsigned c)
        {
            state_[z] = c;
        }

        void clear()
        {
            memset(state_,0,4);
        }

        unsigned get()
        {
            if(state_[3]) return state_[3];
            if(state_[2]) return state_[2];
            if(state_[1]) return state_[1];
            if(state_[0]) return state_[0];
            return 0;
        }

        unsigned char state_[4];
    };

    struct grid_t: virtual pic::lckobject_t
    {
        void set(const arranger::colrow_t &cr,unsigned z,unsigned colour)
        {
            if(cr.second==~0U)
                columns_[cr.first].set(z,colour);
            else
                cells_[cr].set(z,colour);
        }

        unsigned get(const arranger::colrow_t &cr)
        {
            pic::lckmap_t<arranger::colrow_t,cell_t>::nbtype::iterator i = cells_.find(cr);
            if(i!=cells_.end() && i->second.get())
                return i->second.get();

            pic::lckmap_t<unsigned,cell_t>::nbtype::iterator i2 = columns_.find(cr.first);
            if(i2!=columns_.end())
                return i2->second.get();

            return 0;
        }
        
        void clear()
        {
            pic::lckmap_t<arranger::colrow_t,cell_t>::nbtype::iterator i;
            for(i=cells_.begin(); i!= cells_.end(); i++)
            {
                i->second.clear();
            }

            pic::lckmap_t<unsigned,cell_t>::nbtype::iterator i2;
            for(i2=columns_.begin(); i2!=columns_.end(); i2++)
            {
                i2->second.clear();
            }
        }

        pic::lckmap_t<arranger::colrow_t,cell_t>::nbtype cells_;
        pic::lckmap_t<unsigned,cell_t>::nbtype columns_;
    };


    struct scroller_t: virtual pic::lckobject_t
    {
        scroller_t(): value_(0),integrate_(0.f),adjusted_(false)
        {
        }

        void adjust(float f,int dir)
        {
            if(f<THRESHOLD_OFF)
            {
                adjusted_ = false;
                return;
            }

            if(f<THRESHOLD_ONE)
            {
                return;
            }

            if(!adjusted_)
            {
                integrate_ = 0.0;
                adjusted_ = true;
                value_ = std::max(0,value_+dir);
                return;
            }

            if(f>THRESHOLD_MANY)
            {
                integrate_ += FACTOR*(f-THRESHOLD_MANY);
                if(integrate_ > 1.f)
                {
                    integrate_ = 0.f;
                    value_ = std::max(0,value_+dir);
                }
                return;
            }
        }

        void reset()
        {
            value_=0;
            integrate_=0.f;
            adjusted_=false;
        }

        int value_;
        float integrate_;
        bool adjusted_;
    };

    struct controller_t: virtual pic::tracked_t, virtual pic::lckobject_t
    {
        controller_t(arranger::view_t::impl_t *i,unsigned n);
        virtual ~controller_t() { tracked_invalidate(); }
        virtual void key_active(const arranger::colrow_t &cr,const piw::data_nb_t &) {}
        virtual void key_data(const arranger::colrow_t &cr,unsigned,const piw::data_nb_t &) {}
        virtual void clear() {}
        void light(const arranger::colrow_t &,unsigned z,unsigned);

        arranger::view_t::impl_t *parent_;
        grid_t grid_;
        unsigned index_;
    };

    struct evt_controller_t: controller_t, virtual public pic::lckobject_t
    {
        evt_controller_t(arranger::view_t::impl_t *v,unsigned n,arranger::model_t *p): controller_t(v,n),model_(p)
        {
            set_event_ = model_->set_event();
            model_->event_set(piw::change_nb_t::method(this,&evt_controller_t::event_set));
        }

        void key_active(const arranger::colrow_t &cr, const piw::data_nb_t &d)
        {
            if(!d.is_bool() || !d.as_bool())
                return;

            if(model_->get_event(cr,0))
                set_event_(piw::makenull_nb(arranger::encode(cr)));
            else
                set_event_(piw::makefloat_nb(0.f,arranger::encode(cr)));
        }

        void event_set(const piw::data_nb_t &d)
        {
            arranger::colrow_t cr = arranger::decode(d.time());
            light(cr,0,d.is_float()?CLR_RED:CLR_OFF);
        }

        void clear()
        {
            model_->clear_events();
            grid_.clear();
        }

        arranger::model_t *model_;
        piw::change_nb_t set_event_;
    };

    struct loop_controller_t;

    struct marker_t: virtual pic::tracked_t, virtual public pic::lckobject_t
    {
        marker_t(controller_t *i,const piw::change_nb_t &s,unsigned l,unsigned c): ctl_(i), set_(s), colour_(c), column_(l)
        {
        }

        ~marker_t()
        {
            tracked_invalidate();
        }

        void position_set(const piw::data_nb_t &d)
        {
            if(d.is_long())
            {
                light(false);
                column_ = d.as_long();
                light(true);
            }
        }

        void light(bool b)
        {
            ctl_->light(std::make_pair(column_,~0U),2,b?colour_:CLR_OFF);
        }

        controller_t *ctl_;
        piw::change_nb_t set_;
        unsigned colour_;
        unsigned column_;
    };

    struct loop_controller_t: controller_t, piw::thing_t, virtual public pic::lckobject_t
    {
        loop_controller_t(arranger::view_t::impl_t *v,unsigned n,arranger::model_t *p): controller_t(v,n),model_(p),start_(this,p->set_loopstart(),p->get_loopstart(),CLR_RED),end_(this,p->set_loopend(),p->get_loopend(),CLR_ORANGE),position_(this,p->set_position(),0,CLR_GREEN),blink_(false),selecting_(0)
        {
            model_->loopstart_set(piw::change_nb_t::method(&start_,&marker_t::position_set));
            model_->loopend_set(piw::change_nb_t::method(&end_,&marker_t::position_set));

            piw::tsd_thing(this);
            start_.light(true);
            end_.light(true);
        }

        ~loop_controller_t()
        {
            tracked_invalidate();
        }

        void key_active(const arranger::colrow_t &cr,const piw::data_nb_t &d)
        {
            if(!d.is_bool() || !d.as_bool())
                return;

            if(!selecting_)
            {
                if(cr.first==start_.column_)
                {
                    pic::logmsg() << "selecting start";
                    selecting_=&start_;
                }
                else if(cr.first==end_.column_)
                {
                    pic::logmsg() << "selecting end";
                    selecting_=&end_;
                }
                else
                {
                    pic::logmsg() << "selecting position";
                    position_.column_=cr.first;
                    selecting_=&position_;
                }

                timer_fast(200);

                return;
            }

            pic::logmsg() << "selection set to " << cr.first;
            selecting_->set_(piw::makelong_nb(cr.first,d.time()));
            selecting_->light(selecting_!=&position_);
            selecting_=0;
            cancel_timer_fast();
        }

        void thing_timer_fast()
        {
            selecting_->light(blink_);
            blink_=!blink_;
        }

        arranger::model_t *model_;
        marker_t start_,end_,position_;
        bool blink_;
        marker_t *selecting_;
    };

    struct vp_wire_t;
    struct vp_signal_t: piw::fastdata_t, virtual public pic::lckobject_t
    {
        vp_signal_t(vp_wire_t *,unsigned);
        bool fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q) { ping(d.time(),q); return true; }
        bool fastdata_receive_data(const piw::data_nb_t &);

        vp_wire_t *wire_;
        unsigned signal_;
    };

    struct vp_wire_t: piw::wire_t, piw::event_data_sink_t, virtual pic::lckobject_t
    {
        vp_wire_t(arranger::view_t::impl_t *i, const piw::event_data_source_t &es);
        ~vp_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned s,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);

        arranger::view_t::impl_t *parent_;
        piw::dataholder_nb_t active_;
        vp_signal_t pressure_,roll_,yaw_,key_;
        piw::data_nb_t id_;
    };

    struct lwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual pic::lckobject_t, virtual pic::counted_t
    {
        lwire_t(unsigned id, const piw::coordinate_t &coordinate) : piw::event_data_source_real_t(piw::pathone(id,0)), id_(id), coordinate_(coordinate), colour_(0) {}
        ~lwire_t() {  source_shutdown(); disconnect(); }

        void source_ended(unsigned seq)
        {
        }

        void light(unsigned c)
        {
            if(c==colour_)
                return;

            if(c)
            {
                if(colour_)
                {
                    unsigned long long t = piw::tsd_time();
                    output_.add_value(1,piw::makefloat_bounded_nb(5,0,0,c,t));
                }
                else
                {
                    output_ = piw::xevent_data_buffer_t(3,PIW_DATAQUEUE_SIZE_TINY);
                    unsigned long long t = piw::tsd_time();
                    output_.add_value(1,piw::makefloat_bounded_nb(5,0,0,c,t));
                    output_.add_value(2,coordinate_.make_data_nb(t));
                    source_start(0,piw::pathone_nb(id_,t),output_);
                }
            }
            else
            {
                if(colour_)
                {
                    source_end(piw::tsd_time());
                }
            }

            colour_ = c;
        }

        const unsigned id_;
        const piw::coordinate_t coordinate_;
        unsigned colour_;
        piw::xevent_data_buffer_t output_;
    };
}

struct arranger::view_t::impl_t: piw::root_ctl_t, piw::decode_ctl_t, piw::thing_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(model_t *m, const piw::cookie_t &lo): model_(m), decoder_(this), position_(0), control_keys_(0), doubletap_(DEFAULT_DOUBLETAP), clear_key_time_(0), doubletap_set_(piw::changelist_nb())
    {
        active_mode_ = 0;
        model_->position_set(piw::change_nb_t::method(this,&impl_t::position_set));
        model_->playstop_set(piw::change_nb_t::method(this,&impl_t::playstop_set));
        playstop_ = model_->set_playstop();

        modes_[0] = std::auto_ptr<controller_t>(new evt_controller_t(this,0,model_));
        modes_[1] = std::auto_ptr<controller_t>(new loop_controller_t(this,1,model_)); 
        modes_[2] = std::auto_ptr<controller_t>(new controller_t(this,2));
        modes_[3] = std::auto_ptr<controller_t>(new controller_t(this,3));
        modes_[4] = std::auto_ptr<controller_t>(new controller_t(this,4));
        modes_[5] = std::auto_ptr<controller_t>(new controller_t(this,5));
        connect(lo);
        piw::tsd_thing(this);
    }

    ~impl_t()
    {
        tracked_invalidate();
    }

    void set_clock(bct_clocksink_t *c) {}
    void set_latency(unsigned l) {} 
    piw::wire_t *wire_create(const piw::event_data_source_t &es) { return new vp_wire_t(this,es); }

    void control_change(const piw::data_nb_t &d)
    {
        if(d.is_dict())
        {
            piw::data_nb_t v = d.as_dict_lookup("courselen");
            if(v.is_tuple())
            {
                decode_courses(v);
                enqueue_slow_nb(v);
            }
        }
    }

    void set_doubletap(const piw::data_nb_t &d)
    {
        doubletap_=d.as_float();
        doubletap_set_(d);
    }

    void thing_dequeue_slow(const piw::data_t &d)
    {
        lwires_.alternate().clear();

        unsigned index = 1;
        for(unsigned x=0; x<d.as_tuplelen(); ++x)
        {
            for(int y=0; y<d.as_tuple_value(x).as_long(); ++y)
            {
                pic::ref_t<lwire_t> lw = pic::ref(new lwire_t(index,piw::coordinate_t(x+1,y+1)));
                lwires_.alternate().push_back(lw);
                connect_wire(lw.ptr(),lw->source());
                ++index;
            }
        }

        lwires_.exchange();
        trigger_fast();
    }

    void thing_trigger_fast()
    {
        draw();
        draw_marker(true);
    }

    void decode_courses(const piw::data_nb_t &courselen)
    {
        rect2k_.clear();

        pic::lckvector_t<unsigned>::nbtype courses;

        unsigned dictlen = courselen.as_tuplelen();
        if(0 == dictlen)
            return;

        courses.reserve(5);

        for(unsigned i = 0; i < dictlen; ++i)
        {
            courses.push_back(courselen.as_tuple_value(i).as_long());
        }

        pic::lckvector_t<unsigned>::nbtype::const_iterator ci,ce;
        ci = courses.begin();
        ce = courses.end();

        // first course is control keys
        control_keys_ = *ci;
        pic::logmsg() << control_keys_ << " control keys";
        ++ci;

        unsigned columns = *std::min_element(ci,ce);
        unsigned row = 0;
        unsigned key = 1+control_keys_;
        for(; ci != ce; ++ci)
        {
            unsigned column = 0;
            while(column<columns)
            {
                unsigned k = key+column;
                colrow_t cr(std::make_pair(column,row));

                rect2k_[cr] = k;

                ++column;
            }
            key += *ci;
            ++row;
        }
    }

    void mode(const piw::data_nb_t &d)
    {
        if(d.is_long())
        {
            long l = d.as_long();
            if(l>=0 && l<MODES)
                activate_mode(l);
        }
    }

    void lights_off()
    {
        pic::flipflop_t<std::vector<pic::ref_t<lwire_t> > >::guard_t g(lwires_);
        std::vector<pic::ref_t<lwire_t> >::const_iterator li,le;
        li = g.value().begin();
        le = g.value().end();
        for(; li!=le; ++li)
        {
            lwire_t *lw = const_cast<lwire_t *>(li->ptr());
            lw->light(CLR_OFF);
        }
    }

    colrow_t view2grid(const colrow_t &cr) const
    {
        return std::make_pair(cr.first+cpos_.value_,cr.second+rpos_.value_);
    }

    colrow_t grid2view(const colrow_t &cr) const
    {
        return std::make_pair(cr.first-cpos_.value_,cr.second-rpos_.value_);
    }

    void light(const colrow_t &cr)
    {
        if(cr.second==~0U)
        {
            colrow_t vcr = grid2view(std::make_pair(cr.first,0));
            vcr.second = 0;

            pic::lckmap_t<arranger::colrow_t,unsigned>::nbtype::const_iterator i = rect2k_.lower_bound(vcr);

            while(i->first.first==vcr.first)
            {
                colrow_t gcr(view2grid(i->first));
                unsigned c = modes_[active_mode_]->grid_.get(gcr);
                light1(i->second,c);
                ++i;
            }
        }
        else
        {
            colrow_t vcr = grid2view(cr);
            unsigned c = modes_[active_mode_]->grid_.get(cr);
            light1(view2key(vcr),c);
        }
    }

    unsigned grid2key(const colrow_t &gcr)
    {
        return view2key(grid2view(gcr));
    }

    unsigned view2key(const colrow_t &vcr)
    {
        pic::lckmap_t<arranger::colrow_t,unsigned>::nbtype::const_iterator i = rect2k_.find(vcr);
        return (i!=rect2k_.end()) ? i->second : 0;
    }

    void light1(unsigned k,unsigned colour)
    {
        if(!k)
            return;

        k -= 1;

        pic::flipflop_t<std::vector<pic::ref_t<lwire_t> > >::guard_t g(lwires_);

        if(k<g.value().size())
        {
            lwire_t *lw = const_cast<lwire_t *>(g.value()[k].ptr());
            lw->light(colour);
        }
    }

    void key_active(int course, int key, const piw::data_nb_t &d)
    {
        //pic::logmsg() << "key active " << course << ", " << key << ", " << d;
        if(1 == course)
        {
            if(d.is_bool() && d.as_bool())
            {
                switch(key)
                {
                    case KEY_ARRANGER_ONOFF:
                        playstop_(piw::makebool_nb(!model_->is_playing(),0));
                        break;
                    case KEY_EVENT_MODE:
                        change_mode(0);
                        break;
                    case KEY_LOOPDURATION_MODE:
                        change_mode(1);
                        break;
                    case KEY_RESETWINDOW:
                        reset_scrollers();
                        break;
                    case KEY_CLEARALL:
                        if(clear_key_time_)
                        {
                            if(d.time()-clear_key_time_ < doubletap_*1000000)
                            {
                                clear_events();
                                clear_key_time_=0;
                                cancel_timer_fast();
                                light1(KEY_CLEARALL,CLR_OFF);
                            }
                        }
                        else
                        {
                            clear_key_time_=d.time();
                            timer_fast(doubletap_*1000);
                            light1(KEY_CLEARALL,CLR_RED);
                        }
                        break;
                }
            }
            return;
        }

        colrow_t k(std::make_pair(key-1,course-2));
        pic::lckmap_t<colrow_t,unsigned>::nbtype::const_iterator i;

        if((i=rect2k_.find(k))==rect2k_.end())
            return;

        pic::logmsg() << "col " << k.first << " row " << k.second;
        seq_active(k,d);
    }

    void thing_timer_fast()
    {
        clear_key_time_=0;
        light1(KEY_CLEARALL,CLR_OFF);
    }

    void key_data(int course, int key, unsigned s, const piw::data_nb_t &d)
    {
        //pic::logmsg() << "key data " << course << ", " << key << ", " << s << ", " << d;
        if(1 == course)
        {
            switch(key)
            {
                case KEY_SCROLL_RIGHT:
                case KEY_SCROLL_LEFT:
                case KEY_SCROLL_DOWN:
                case KEY_SCROLL_UP:
                    ctrl_data(key-1,s,d);
                    break;
            }
            return;
        }

        colrow_t k(std::make_pair(key-1,course-2));
        pic::lckmap_t<colrow_t,unsigned>::nbtype::const_iterator i;

        if((i=rect2k_.find(k))==rect2k_.end())
            return;

        seq_data(k,s,d);
    }

    void change_mode(unsigned m)
    {
        if(active_mode_==m)
        {
            activate_mode(0);
        }
        else
        {
            activate_mode(m);
        }
    }

    void playstop_set(const piw::data_nb_t &d)
    {
        pic::logmsg() << "playstop set to " << d.as_bool();
        light1(KEY_ARRANGER_ONOFF,d.as_bool()?CLR_GREEN:CLR_OFF);
    }

    void seq_active(const colrow_t &cr,const piw::data_nb_t &d)
    {
        colrow_t tcr = view2grid(cr);
        pic::logmsg() << "seq active " << cr.first << "," << cr.second << " -> " <<
            tcr.first << "," << tcr.second;
        modes_[active_mode_]->key_active(tcr,d);
    }

    void ctrl_data(unsigned k,unsigned s,const piw::data_nb_t &d)
    {
        if(s!=1)
            return;

        int c = cpos_.value_;
        int r = rpos_.value_;

        float f = d.as_renorm(0,1,0);
        switch(k)
        {
            case 0: rpos_.adjust(f,1); break;
            case 1: rpos_.adjust(f,-1); break;
            case 2: cpos_.adjust(f,1); break;
            case 3: cpos_.adjust(f,-1); break;
        }

        if(c!=cpos_.value_ || r!=rpos_.value_)
        {
            draw();
            draw_marker(true);
        }
    }

    void reset_scrollers()
    {
        rpos_.reset();
        cpos_.reset();
        draw();
        draw_marker(true);
    }

    void clear_events()
    {
        modes_[0]->clear();
        draw();
        draw_marker(true);
    }

    void seq_data(const colrow_t &cr,unsigned s,const piw::data_nb_t &d)
    {
        modes_[active_mode_]->key_data(cr,s,d);
    }

    void position_set(const piw::data_nb_t &d)
    {
        if(!d.is_long())
            return;

        draw_marker(false);
        position_ = d.as_long();
        draw_marker(true);
    }

    void draw_marker(bool d)
    {
        arranger::colrow_t vcr = grid2view(std::make_pair(position_,0));

        vcr.second = 0;
        pic::lckmap_t<arranger::colrow_t,unsigned>::nbtype::const_iterator i,e;
        i=rect2k_.lower_bound(vcr);
        e=rect2k_.end();
        while(i!=e && i->first.first==vcr.first)
        {
            light1(i->second,d?CLR_GREEN:modes_[active_mode_]->grid_.get(view2grid(i->first)));
            ++i;
        }
    }

    void draw()
    {
        pic::lckmap_t<arranger::colrow_t,unsigned>::nbtype::const_iterator i, e;
        i = rect2k_.begin();
        e = rect2k_.end();
        for(; i!=e; ++i)
        {
            light1(i->second,modes_[active_mode_]->grid_.get(view2grid(i->first)));
        }
    }

    void activate_mode(unsigned i)
    {
        if(i==active_mode_)
            return;

        active_mode_=i;
        lights_off();
        draw();
        draw_marker(true);
        playstop_set(piw::makebool_nb(model_->is_playing(),0));

        if(active_mode_)
            light1(active_mode_+6,CLR_RED);
    }

    model_t *model_;

    piw::decoder_t decoder_;

    scroller_t rpos_;
    scroller_t cpos_;

    pic::lckmap_t<colrow_t,unsigned>::nbtype rect2k_;

    std::auto_ptr<controller_t> modes_[MODES];
    unsigned active_mode_;

    pic::flipflop_t<std::vector<pic::ref_t<lwire_t> > > lwires_;
    unsigned position_;

    piw::change_nb_t playstop_;
    unsigned control_keys_;
   
    float doubletap_; 
    unsigned long long clear_key_time_;

    piw::change_nb_t doubletap_set_;
};

controller_t::controller_t(arranger::view_t::impl_t *i,unsigned n): parent_(i), index_(n)
{
}

void controller_t::light(const arranger::colrow_t &cr, unsigned z, unsigned colour)
{
    grid_.set(cr,z,colour);
    if(index_==parent_->active_mode_)
        parent_->light(cr);
}

vp_wire_t::vp_wire_t(arranger::view_t::impl_t *i, const piw::event_data_source_t &es) : parent_(i), pressure_(this,1), roll_(this,2), yaw_(this,3), key_(this,5)
{
    subscribe(es);
}

void vp_wire_t::invalidate()
{
    if(parent_)
    {
        unsubscribe();
        disconnect();
        parent_=0;
    }
}

void vp_wire_t::event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b)
{
    active_.clear_nb();
    id_ = id;
    key_.send_fast(id,b.signal(SIG_KEY));
    pressure_.send_fast(id,b.signal(SIG_PRESSURE));
    roll_.send_fast(id,b.signal(SIG_ROLL));
    yaw_.send_fast(id,b.signal(SIG_YAW));
}

void vp_wire_t::event_buffer_reset(unsigned s,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    switch(s)
    {
        case SIG_PRESSURE:
            pressure_.send_fast(id_,n);
            break;
        case SIG_ROLL:
            roll_.send_fast(id_,n);
            break;
        case SIG_YAW:
            yaw_.send_fast(id_,n);
            break;
        case SIG_KEY:
            key_.send_fast(id_,n);
            break;
    }
}

bool vp_wire_t::event_end(unsigned long long t)
{
    float course, key;
    if(piw::decode_key(active_.get(),0,0,&course,&key))
    {
        parent_->key_active(course,key,piw::makebool_nb(false,t));
    }
    active_.clear_nb();
    id_ = piw::makenull_nb(t);
    return true;
}

vp_signal_t::vp_signal_t(vp_wire_t *w, unsigned s): fastdata_t(PLG_FASTDATA_SENDER), wire_(w), signal_(s)
{
    piw::tsd_fastdata(this);
    enable(true,false,false);
}

bool vp_signal_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    float course, key;
    if(SIG_KEY == signal_)
    {
        piw::hardness_t hardness = piw::KEY_LIGHT;
        if(piw::decode_key(d,0,0,&course,&key,&hardness) && hardness != piw::KEY_LIGHT)
        {
            wire_->active_.set_nb(d);
            wire_->parent_->key_active(course,key,piw::makebool_nb(true,d.time()));
        }
    }
    else if(!wire_->active_.is_empty())
    {
        if(piw::decode_key(wire_->active_.get(),0,0,&course,&key))
        {
            wire_->parent_->key_data(course,key,signal_,d);
        }
    }
    return true;
}

arranger::view_t::view_t(model_t *p,const piw::cookie_t &lo) : impl_(new impl_t(p,lo))
{
}

arranger::view_t::~view_t()
{
    delete impl_;
}

piw::cookie_t arranger::view_t::cookie()
{
    return impl_->decoder_.cookie();
}

piw::change_nb_t arranger::view_t::control()
{
    return piw::change_nb_t::method(impl_,&arranger::view_t::impl_t::control_change);
}

void arranger::view_t::clear_events()
{
    return impl_->clear_events();
}

piw::change_nb_t arranger::view_t::set_doubletap()
{
    return piw::change_nb_t::method(impl_,&arranger::view_t::impl_t::set_doubletap);
}

void arranger::view_t::doubletap_set(const piw::change_nb_t &c)
{
    piw::changelist_connect_nb(impl_->doubletap_set_,c);
}
