
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


#include "arranger_model.h"
#include "arranger_colrow.h"
#include <piw/piw_clockclient.h>
#include <piw/piw_tsd.h>
#include <math.h>
#include <algorithm>
#include <plg_arranger/piarranger_exports.h>

namespace
{
    typedef std::pair<float,unsigned> point_t;
    typedef pic::lckvector_t<point_t>::nbtype column_t;
    typedef column_t::iterator citer_t;

    struct PIARRANGER_DECLSPEC_CLASS grid_t: virtual pic::lckobject_t
    {
        void set_event(const arranger::colrow_t &cr,float f)
        {
            unsigned c = cr.first;
            unsigned r = cr.second;

            column_t *l = at(c,true);
            if(!l)
                columns_.at(c) = l = new column_t;

            citer_t b = l->begin();
            citer_t e = l->end();
            citer_t i;

            for(i=b; i!=e; ++i)
            {
                if(i->second==r)
                {
                    l->erase(i);
                    break;
                }
            }

            point_t p = std::make_pair(f,r);
            i = std::lower_bound(b,e,p);
            l->insert(i,p);
        }

        ~grid_t()
        {
            pic::lckvector_t<column_t *>::nbtype::iterator i,e;
            i = columns_.begin();
            e = columns_.end();
            for(; i!=e; ++i)
                delete *i;
        }

        column_t *at(unsigned c,bool extend)
        {
            if(c<columns_.size())
                return columns_.at(c);

            if(!extend)
                return 0;

            columns_.resize(c+1);
            return columns_.at(c);
        }

        void del_event(const arranger::colrow_t &cr)
        {
            unsigned c = cr.first;
            unsigned r = cr.second;

            column_t *l = at(c,false);
            if(!l)
                return;

            citer_t b = l->begin();
            citer_t e = l->end();
            citer_t i;

            for(i=b; i!=e; ++i)
            {
                if(i->second==r)
                {
                    l->erase(i);
                    break;
                }
            }

            if(l->empty())
            {
                columns_.at(c) = 0;
                delete l;
            }
        }

        bool get_event(const arranger::colrow_t &cr,float *f)
        {
            unsigned c = cr.first;
            unsigned r = cr.second;

            column_t *l = at(c,false);
            if(!l)
                return false;

            citer_t b = l->begin();
            citer_t e = l->end();
            citer_t i;

            for(i=b; i!=e; ++i)
            {
                if(i->second==r)
                {
                    if(f) *f = i->first;
                    return true;
                }
            }

            return false;
        }

        void clear_events()
        {
            columns_.clear();
        }

        pic::lckvector_t<column_t *>::nbtype columns_;
    };
}

struct arranger::model_t::impl_t: piw::decode_ctl_t, piw::wire_t, piw::event_data_sink_t, piw::clocksink_t, virtual pic::tracked_t
{
    impl_t(piw::clockdomain_ctl_t *d) : decoder_(this), interp_(1), upstream_(0), event_set_(piw::changelist_nb()), loopstart_set_(piw::changelist_nb()), loopend_set_(piw::changelist_nb()), position_set_(piw::changelist_nb()), stepnumerator_set_(piw::changelist_nb()), stepdenominator_set_(piw::changelist_nb()), playstop_set_(piw::changelist_nb()), loopstart_(0), loopend_(15), stepnumerator_(1.f), stepdenominator_(2.f), transport_(false), clock_(0), step_(0), cindex_(0), last_time_(0), count_(0), playing_(true)
    {
        setup_loop();
        d->sink(this,"arranger");
    }

    ~impl_t()
    {
        tracked_invalidate();
        invalidate();
    }

    void setup_loop()
    {
        int l = loopend_-loopstart_;
        direction_ = (l<0) ? -1:1;
        frac_ = 0.f;
        cindex_ = 0;
        row_ = ~0U;
    }

    void set_clock(bct_clocksink_t *c)
    {
        if(c==upstream_)
            return;
        if(upstream_)
            remove_upstream(upstream_);
        upstream_ = c;
        if(upstream_)
            add_upstream(upstream_);
    }

    void set_latency(unsigned) {}
    void wire_closed() { invalidate(); }

    void invalidate()
    {
        tick_disable();
        unsubscribe();
    }

    piw::wire_t *wire_create(const piw::event_data_source_t &es)
    {
        tick_enable(true);
        subscribe(es);
        return this;
    }

    void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &ei)
    {
        iterator_ = ei.iterator();
        iterator_->reset(1,id.time());
        iterator_->reset(2,id.time());
        tick_suppress(false);
    }

    void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
    {
        iterator_->set_signal(sig,n);
        iterator_->reset(sig,t);
    }

    void clkupdate(unsigned long long t)
    {
        unsigned sig;
        piw::data_nb_t d;
        while(iterator_->next(3,sig,d,t))
        {
            switch(sig)
            {
                case 1: interp_.recv_clock(0,d); break;
                case 2: transport_ = d.as_norm()!=0; break;
            }
        }
    }

    float beats2steps(float s)
    {
        return s/(stepnumerator_/stepdenominator_);
    }

    float steps2beats(float b)
    {
        return b*(stepnumerator_/stepdenominator_);
    }

    bool event_end(unsigned long long t)
    {
        transport_ = false;
        tick_suppress(true);
        reset();
        return true;
    }

    void reset()
    {
        clock_ = 0;
        last_time_ = 0;
        step_ = 0;
        frac_ = 0.f;
        count_ = 0;
    }

    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        bool ot = transport_;

        clkupdate(t);

        if(ot!=transport_)
        {
            pic::logmsg() << "transport is " << (transport_?"running":"stopped");

            playstop_set_(piw::makebool_nb(playing_,t));

            if(!transport_)
            {
                pic::logmsg() << "transport off, suppressing";
                tick_suppress(true);
                reset();
                return;
            }
        }

        float b = interp_.interpolate_clock(0,t);

        if(count_<2)
        {
            ++count_;
        }
        else
        {
            trigger(b);
        }

        last_time_ = t;
    }

    void sync(float b)
    {
        clock_ = (unsigned long)beats2steps(b);
    }

    void trigger(float b)
    {
        float s = beats2steps(b);
        float c;

        pic::flipflop_t<pic::lckvector_t<piw::change_t>::lcktype>::guard_t g(targets_);

        while((c=clock())<s)
        {
            update();

            if(row_<g.value().size() && playing_)
            {
                unsigned long long tt = interp_.interpolate_time(0,steps2beats(c),0);
                g.value().at(row_)(piw::makebool(true,tt));
                g.value().at(row_)(piw::makebool(false,tt+1));
            }

            advance();
        }
    }

    void advance()
    {
        if(direction_>0)
        {
            forwards();
        }
        else
        {
            backwards();
        }
    }

    int position()
    {
        return loopstart_+direction_*step_;
    }

    float clock()
    {
        return (float)clock_+frac_;
    }

    void update()
    {
        column_t *l = grid_.at(position(),false);
        if(l && cindex_>=0 && cindex_<(int)l->size())
        {
            frac_ = l->at(cindex_).first;
            row_  = l->at(cindex_).second;
        }
        else
        {
            frac_ = 0.f;
            row_ = ~0U;
        }
    }

    void forwards()
    {
        column_t *l = grid_.at(position(),false);

        ++cindex_;

        if(!l || cindex_>=(int)l->size())
        {
            position_set_(piw::makelong_nb(position(),0));

            ++step_;
            if(position()>loopend_ || position()<loopstart_)
                step_ = 0;
            ++clock_;
            l = grid_.at(position(),false);
            cindex_ = 0;
        }
    }

    void backwards()
    {
        column_t *l = grid_.at(position(),false);

        --cindex_;

        if(!l || cindex_<0)
        {
            position_set_(piw::makelong_nb(position(),0));

            ++step_;
            if(position()<loopend_ || position()>loopstart_)
                step_ = 0;
            ++clock_;
            l = grid_.at(position(),false);
            if(l)
                cindex_ = (int)l->size()-1;
        }
    }

    void set_target(unsigned r,const piw::change_t &c)
    {
        pic::logmsg() << "target set on row " << r;

        if(targets_.alternate().size()<r+1)
            targets_.alternate().resize(r+1);

        targets_.alternate()[r] = c;
        targets_.exchange();
    }

    void clear_target(unsigned r)
    {
        if(targets_.alternate().size()<r+1)
        {
            targets_.alternate()[r] = piw::change_t();
            targets_.exchange();
        }
    }

    void set_event(const piw::data_nb_t &d)
    {
        pic::logmsg() << "model set_event " << d;
        colrow_t cr = decode(d.time());
        if(d.is_float())
        {
            float f = d.as_float();
            grid_.set_event(cr,f);
            event_set_(d);
        }
        else
        {
            grid_.del_event(cr);
            event_set_(piw::makenull_nb(encode(cr)));
        }
    }

    bool get_event(const colrow_t &cr,float *f)
    {
        return grid_.get_event(cr,f);
    }

    void clear_events()
    {
        pic::logmsg() << "model clear_events";
        grid_.clear_events();
    }

    void set_loopstart(const piw::data_nb_t &d)
    {
        pic::logmsg() << "set_loopstart " << piw::fullprinter_t<piw::data_nb_t>(d);
        if(d.is_long())
        {
            int l = d.as_long();
            if(l!=loopstart_ && l!=loopend_)
            {
                loopstart_ = l;
                setup_loop();
            }

            if(loopstart_==l)
            {
                loopstart_set_(d);
            }
        }
    }

    void set_loopend(const piw::data_nb_t &d)
    {
        pic::logmsg() << "set_loopend " << piw::fullprinter_t<piw::data_nb_t>(d);
        if(d.is_long())
        {
            int l = d.as_long();
            if(l!=loopstart_ && l!=loopend_)
            {
                loopend_ = l;
                setup_loop();
            }

            if(loopend_==l)
            {
                loopend_set_(d);
            }
        }
    }

    void set_position(const piw::data_nb_t &d)
    {
        if(!d.is_long())
            return;

        int p = d.as_long();

        if(direction_>0)
        {
            p = std::min(loopend_,p);
            p = std::max(loopstart_,p);
        }
        else
        {
            p = std::max(loopend_,p);
            p = std::min(loopstart_,p);
        }

        pic::logmsg() << "position setting to " << p << " (got " << d << ")";

        step_ = direction_*(p-loopstart_);
        cindex_ = 0;
        row_ = ~0U;
        frac_ = 0.f;
        //position_set_(d);
    }

    void set_stepnumerator(const piw::data_nb_t &d)
    {
        if(!d.is_float())
            return;

        if(d.as_float()==stepnumerator_)
            return;

        stepnumerator_ = d.as_float();
        sync(interp_.interpolate_clock(0,last_time_));
        stepnumerator_set_(d);
    }

    void set_stepdenominator(const piw::data_nb_t &d)
    {
        if(!d.is_float())
            return;

        if(d.as_float()==stepdenominator_)
            return;

        stepdenominator_ = d.as_float();
        sync(interp_.interpolate_clock(0,last_time_));
        stepdenominator_set_(d);
    }

    void playstop(const piw::data_nb_t &d)
    {
        if(!d.is_bool())
            return;

        playing_ = d.as_bool();
        playstop_set_(d);
    }

    int gc_traverse(void *v, void *a) const
    {
        int r;

        if((r=event_set_.gc_traverse(v,a))!=0) return r;
        if((r=loopstart_set_.gc_traverse(v,a))!=0) return r;
        if((r=loopend_set_.gc_traverse(v,a))!=0) return r;
        if((r=position_set_.gc_traverse(v,a))!=0) return r;
        if((r=stepnumerator_set_.gc_traverse(v,a))!=0) return r;
        if((r=stepdenominator_set_.gc_traverse(v,a))!=0) return r;
        if((r=playstop_set_.gc_traverse(v,a))!=0) return r;

        pic::lckvector_t<piw::change_t>::lcktype::const_iterator i;

        for(i=targets_.current().begin(); i!=targets_.current().end(); i++)
        {
            if((r=i->gc_traverse(v,a))!=0)
            {
                return r;
            }
        }

        return 0;
    }

    int gc_clear()
    {
        event_set_.gc_clear();
        loopstart_set_.gc_clear();
        loopend_set_.gc_clear();
        position_set_.gc_clear();
        stepnumerator_set_.gc_clear();
        stepdenominator_set_.gc_clear();
        playstop_set_.gc_clear();

        targets_.alternate().clear();
        targets_.exchange();

        return 0;
    }

    piw::decoder_t decoder_;
    piw::clockinterp_t interp_;
    bct_clocksink_t *upstream_;

    pic::flipflop_t<pic::lckvector_t<piw::change_t>::lcktype> targets_;

    piw::change_nb_t event_set_;
    piw::change_nb_t loopstart_set_;
    piw::change_nb_t loopend_set_;
    piw::change_nb_t position_set_;
    piw::change_nb_t stepnumerator_set_;
    piw::change_nb_t stepdenominator_set_;
    piw::change_nb_t playstop_set_;

    int loopstart_;
    int loopend_;

    float stepnumerator_,stepdenominator_;
    int direction_;
    bool transport_;

    unsigned long clock_;
    int step_;
    float frac_;
    unsigned row_;

    grid_t grid_;

    int cindex_;
    unsigned long long last_time_;
    unsigned count_;
    bool playing_;

    piw::xevent_data_buffer_t::iter_t iterator_;
};

arranger::model_t::model_t(piw::clockdomain_ctl_t *d) : impl_(new impl_t(d))
{
}

arranger::model_t::~model_t()
{
    delete impl_;
}

bct_clocksink_t *arranger::model_t::get_clock()
{
    return impl_->decoder_.get_clock();
}

piw::cookie_t arranger::model_t::cookie()
{
    return impl_->decoder_.cookie();
}

void arranger::model_t::set_target(unsigned r, const piw::change_t &target) { impl_->set_target(r,target); }
void arranger::model_t::clear_target(unsigned r) { impl_->clear_target(r); }

piw::change_nb_t arranger::model_t::set_event() { return piw::change_nb_t::method(impl_,&impl_t::set_event); }
void arranger::model_t::event_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->event_set_,c); }
bool arranger::model_t::get_event(const colrow_t &cr,float *f) { return impl_->get_event(cr,f); }

void arranger::model_t::clear_events() { impl_->clear_events(); }

piw::change_nb_t arranger::model_t::set_loopstart() { return piw::change_nb_t::method(impl_,&impl_t::set_loopstart); }
void arranger::model_t::loopstart_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->loopstart_set_,c); }
unsigned arranger::model_t::get_loopstart() { return impl_->loopstart_; }

piw::change_nb_t arranger::model_t::set_loopend() { return piw::change_nb_t::method(impl_,&impl_t::set_loopend); }
void arranger::model_t::loopend_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->loopend_set_,c); }
unsigned arranger::model_t::get_loopend() { return impl_->loopend_; }

piw::change_nb_t arranger::model_t::set_position() { return piw::change_nb_t::method(impl_,&impl_t::set_position); }
void arranger::model_t::position_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->position_set_,c); }
unsigned arranger::model_t::get_position() { return impl_->position(); }

piw::change_nb_t arranger::model_t::set_stepnumerator() { return piw::change_nb_t::method(impl_,&impl_t::set_stepnumerator); }
void arranger::model_t::stepnumerator_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->stepnumerator_set_,c); }

piw::change_nb_t arranger::model_t::set_stepdenominator() { return piw::change_nb_t::method(impl_,&impl_t::set_stepdenominator); }
void arranger::model_t::stepdenominator_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->stepdenominator_set_,c); }

piw::change_nb_t arranger::model_t::set_playstop() { return piw::change_nb_t::method(impl_,&impl_t::playstop); }
void arranger::model_t::playstop_set(const piw::change_nb_t &c) { piw::changelist_connect_nb(impl_->playstop_set_,c); }

bool arranger::model_t::is_playing() { return impl_->playing_; }

int arranger::model_t::gc_traverse(void *v, void *a) const { return impl_->gc_traverse(v,a); }
int arranger::model_t::gc_clear() { return impl_->gc_clear(); }

