
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

#include <piw/piw_scheduler.h>

#include <piw/piw_fastdata.h>
#include <piw/piw_clock.h>
#include <piw/piw_bundle.h>
#include <piw/piw_tsd.h>
#include <piw/piw_clockclient.h>
#include <piw/piw_slowchange.h>
#include <piw/piw_thing.h>

#include <picross/pic_ref.h>
#include <picross/pic_log.h>

#include <map>
#include <vector>
#include <set>
#include <cmath>

#define ET_NULL 0
#define ET_EDGEGOOD 1
#define ET_EDGEBAD 2
#define ET_MOD 3
#define ET_ZONEGOOD 5
#define ET_ZONEBAD 6

namespace 
{
    typedef piw::event_t::impl_t eventimpl_t;
    typedef piw::scheduler_t::impl_t schedimpl_t;

    //                                      0  1  2  3  4  5  6  7  8  9  10 11 12
    static const unsigned mod_sizes__[] = { 24,24,24,18,24,10,18,7 ,24,18,10,11,24 };

    static unsigned mod_size(unsigned divisor)
    {
        if(divisor < sizeof(mod_sizes__)/sizeof(unsigned))
        {
            return mod_sizes__[divisor];
        }

        return divisor;
    }

    struct fastarg_t
    {
        fastarg_t(unsigned u1_, unsigned u2_, float f1_, float f2_): u1(u1_), u2(u2_), f1(f1_), f2(f2_) {}
        unsigned u1,u2;
        float f1,f2;
    };

    struct clkevent_t;

    struct edge_constraint_t
    {
        edge_constraint_t(eventimpl_t *e, unsigned s, bool l, float v): event_(e), signal_(s), edge_lower_(l), edge_true_(true), edge_value_(v)
        {
        }

        bool applies() const
        {
            return edge_true_;
        }

        void edge_bad(unsigned long long t);
        void edge_good(unsigned long long t);

        void event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value);

        void update_edge(float v)
        {
            if(edge_lower_)
            {
                edge_true_ = (edge_value_ < v);
            }
            else
            {
                edge_true_ = (v <= edge_value_);
            }

            //pic::msg() << "clock " << signal_ << " edge " << edge_value_ << " is " << (edge_true_?"good":"bad") << pic::log;
        }

        eventimpl_t *event_;
        unsigned signal_;
        bool edge_lower_;
        bool edge_true_;
        float edge_value_;
    };

    struct mod_constraint_t
    {
        mod_constraint_t(): armed_(false) {}
        virtual ~mod_constraint_t() {}

        virtual void event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value) = 0;

        bool armed_;
    };

    struct point_constraint_t: mod_constraint_t
    {
        point_constraint_t(eventimpl_t *e, unsigned s, long d, float r): event_(e), signal_(s), mod_divisor(d), mod_remainder(r)
        {
        }

        void event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value);
        void mod(unsigned long long t);

        eventimpl_t *event_;
        unsigned signal_;
        long mod_divisor;
        float mod_remainder;
    };

    struct zone_constraint_t: mod_constraint_t
    {
        zone_constraint_t(eventimpl_t *e, unsigned s, long d, float r1, float r2): event_(e), signal_(s), mod_divisor(d), mod_remainder1(r1), mod_remainder2(r2), zone_true_(false)
        {
        }

        void event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value);

        void zone_good(unsigned long long t);
        void zone_bad(unsigned long long t);

        bool applies() const
        {
            return zone_true_;
        }

        void update_edge(float v)
        {
            float mod = fmod(v,mod_divisor);

            if(mod_remainder1 < mod_remainder2)
            {
                zone_true_ = (mod>=mod_remainder1 && mod<=mod_remainder2);
            }
            else
            {
                zone_true_ = (mod<mod_remainder1 || mod>=mod_remainder2);
            }

            //pic::msg() << "clock " << signal_ << " zone " << mod_remainder1 << "->" << mod_remainder2 << " is " << (zone_true_?"good":"bad") << pic::log;
        }

        eventimpl_t *event_;
        unsigned signal_;
        long mod_divisor;
        float mod_remainder1;
        float mod_remainder2;
        bool zone_true_;
    };

    struct clkevent_t
    {
        clkevent_t(int x,int s, edge_constraint_t *e, float v, unsigned long long t): type(x), signal(s), event(e), mod(0), value(v), time(t) {}
        clkevent_t(int x,int s, point_constraint_t *e, unsigned long long t): type(x), signal(s), event(0), mod(e), value(0), time(t) {}
        clkevent_t(int x,int s, zone_constraint_t *e, unsigned long long t): type(x), signal(s), event(0), zone(e), value(0), time(t) {}
        clkevent_t(const clkevent_t &e): type(e.type), signal(e.signal), event(e.event), mod(e.mod), zone(e.zone), value(e.value), time(e.time) {}
        clkevent_t(): type(ET_NULL), signal(0), event(0), mod(0), value(0),time(0) {}

        int type;
        int signal;
        edge_constraint_t *event;
        point_constraint_t *mod;
        zone_constraint_t *zone;
        float value;
        unsigned long long time;
    };

    struct edge_table_t
    {
        edge_table_t()
        {
        }

        float interp(float value, unsigned long long ubound_time, float ubound_clock)
        {
            unsigned long long usdist = ubound_time-current_timestamp_;
            double clkdist = ubound_clock-current_clock_;
            double ratio = (value-current_clock_)/clkdist;
            return ((double)usdist)*ratio+current_timestamp_;
        }

        void reset(unsigned long long next_stamp, float next_clock)
        {
            current_timestamp_ = next_stamp;
            current_clock_ = next_clock;
        }

        void sweep(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long ubound_time, float ubound_clock)
        {
            //pic::logmsg() << "table sweep time " << current_timestamp_ << "-" << ubound_time << " clk " << current_clock_ << "-" << ubound_clock;
            std::multimap<float, edge_constraint_t *>::iterator ei=table_.lower_bound(current_clock_);

            while(ei!=table_.end() && ei->first<ubound_clock)
            {
                unsigned long long event_time = (unsigned long long)interp(ei->first,ubound_time,ubound_clock);
                ei->second->event(q,event_time,ei->first);
                ei++;
            }

            current_clock_ = ubound_clock;
            current_timestamp_ = ubound_time;
        }

        void insert_edge(float v, edge_constraint_t *x)
        {
            table_.insert(std::make_pair(v,x));
        }

        void remove_edge(float v, edge_constraint_t *x)
        {
            std::multimap<float, edge_constraint_t *>::iterator i,e;

            e=table_.end();
            i=table_.lower_bound(v);

            while(i!=e && i->first == v)
            {
                if(i->second==x)
                {
                    table_.erase(i);
                    break;
                }

                i++;
            }
        }

        std::multimap<float, edge_constraint_t *> table_;
        unsigned long long current_timestamp_;
        float current_clock_;
    };

    struct mod_table_t
    {
        mod_table_t(unsigned size, unsigned long long t, float c): tablesize_(size)
        {
            table_.insert(std::make_pair(tablesize_,std::make_pair(tablesize_,(mod_constraint_t *)0)));
            reset(t,c);
        }

        void add_constraint(mod_constraint_t *mc, float divisor, float remainder)
        {
            mc->armed_ = true;
            for(float i=remainder; i<tablesize_; i+=divisor)
            {
                //pic::msg() << "table size=" << tablesize_ << " i=" << i << " cf=" << current_frame_ << " cr=" << current_remainder_ << " lr=" << last_remainder_ << pic::log;
                table_.insert(std::make_pair(i,std::make_pair(remainder,mc)));
                if(i<=last_remainder_ && i>=current_remainder_)
                {
                    mc->armed_ = false;
                }
            }
        }

        void remove_constraint(mod_constraint_t *mc, float divisor, float remainder)
        {
            for(float i=remainder; i<tablesize_; i+=divisor)
            {
                std::multimap<float,std::pair<float,mod_constraint_t *> >::iterator mi = table_.lower_bound(i);

                while(mi != table_.end() && mi->first==i)
                {
                    if(mi->second.second==mc)
                    {
                        table_.erase(mi);
                        break;
                    }

                    mi++;
                }
            }
        }

        float interp(float value, unsigned long long ubound_time, float ubound_clock)
        {
            unsigned long long usdist = ubound_time-current_timestamp_;
            double clkdist = ubound_clock-current_clock_;
            double ratio = (value-current_clock_)/clkdist;
            return ((double)usdist)*ratio+current_timestamp_;
        }

        void sweep(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long ubound_time, float ubound_clock)
        {
            std::multimap<float,std::pair<float,mod_constraint_t *> >::iterator mi=table_.lower_bound(current_remainder_);
            //pic::logmsg() << "sweep utime: " << ubound_time << " uclk: " << ubound_clock;

            float c = current_clock_;
            while(c < ubound_clock)
            {
                double event_clock = current_frame_*tablesize_+mi->first;
                if(event_clock >= ubound_clock)
                {
                    break;
                }

                if(!mi->second.second)
                {
                    mi=table_.begin();
                    current_frame_+=1;
                    current_remainder_=0;
                    continue;
                }

                if(mi->second.second->armed_)
                {
                    unsigned long long event_time = (unsigned long long)interp(event_clock,ubound_time,ubound_clock);
                    mi->second.second->event(q,event_time,mi->second.first);
                }
                else
                {
                    mi->second.second->armed_ = true;
                }

                mi++;

                current_remainder_=mi->first;
                c = current_frame_*tablesize_+current_remainder_;
            }

            last_remainder_ = ubound_clock-current_frame_*tablesize_;
            current_clock_ = ubound_clock;
            current_timestamp_ = ubound_time;
        }

        void reset(unsigned long long next_stamp, float next_clock)
        {
            current_timestamp_ = next_stamp;
            current_frame_ = (unsigned long)floor(next_clock/tablesize_);
            current_remainder_ = next_clock-current_frame_*tablesize_;
            current_clock_ = current_frame_*tablesize_+current_remainder_;
            last_remainder_=-1;
            //pic::msg() << "table reset size=" << tablesize_ << " frame=" << current_frame_ << " clk=" << next_clock << " cc=" << current_clock_ << " cr=" << current_remainder_ << pic::log;
        }

        std::multimap<float,std::pair<float,mod_constraint_t *> > table_;
        unsigned tablesize_;
        unsigned long long current_timestamp_;
        unsigned long current_frame_;
        float current_clock_;
        float current_remainder_;
        float last_remainder_;
    };

};

struct piw::event_t::impl_t: piw::thing_t
{
    impl_t(const piw::change_nb_t &f, schedimpl_t *s, unsigned c, const piw::change_t &cb): enabled_(false), scheduler_(s), action_(f), clkcount_(c), edge_constraints_(c), point_constraints_(c), zone_constraints_(c), modstatus_(0), edgestatus_(0), zonestatus_(0), havemods_(false)
    {
        piw::fastchange(f)(piw::makefloat_bounded(1,0,0,0,0));
        piw::tsd_thing(this);
        set_slow_dequeue_handler(cb);
    }

    ~impl_t()
    {
        call_disable();
    }

    void clear();
    void lower(unsigned s,float v);
    void upper(unsigned s,float v);
    void modulo(unsigned s,unsigned d, float r);
    void zone(unsigned s,unsigned d, float r1, float r2);
    bool disable();
    void enable();

    void reenable(bool enabled) { if(enabled) enable(); }

    static int __clear(void *e_, void *a_) { ((eventimpl_t *)e_)->clear(); return 0; }
    static int __lower(void *e_, void *a_) { fastarg_t *a = (fastarg_t *)a_; ((eventimpl_t *)e_)->lower(a->u1,a->f1); return 0; }
    static int __upper(void *e_, void *a_) { fastarg_t *a = (fastarg_t *)a_; ((eventimpl_t *)e_)->upper(a->u1,a->f1); return 0; }
    static int __modulo(void *e_, void *a_) { fastarg_t *a = (fastarg_t *)a_; ((eventimpl_t *)e_)->modulo(a->u1,a->u2,a->f1); return 0; }
    static int __zone(void *e_, void *a_) { fastarg_t *a = (fastarg_t *)a_; ((eventimpl_t *)e_)->zone(a->u1,a->u2,a->f1,a->f2); return 0; }
    static int __disable(void *e_, void *a_) { ((eventimpl_t *)e_)->disable(); return 0; }
    static int __enable(void *e_, void *a_) { ((eventimpl_t *)e_)->enable(); return 0; }

    void call_clear() { piw::tsd_fastcall(__clear, this, 0); }
    void call_lower(unsigned signal, float value) { fastarg_t a(signal,0,value,0); piw::tsd_fastcall(__lower, this, &a); }
    void call_upper(unsigned signal, float value) { fastarg_t a(signal,0,value,0); piw::tsd_fastcall(__upper, this, &a); }
    void call_zone(unsigned signal, unsigned divisor, float r1, float r2) { fastarg_t a(signal,divisor,r1,r2); piw::tsd_fastcall(__zone, this, &a); }
    void call_modulo(unsigned signal, unsigned divisor, float remainder) { fastarg_t a(signal,divisor,remainder,0); piw::tsd_fastcall(__modulo, this, &a); }
    void call_enable() { piw::tsd_fastcall(__enable, this, 0); enqueue_slow(piw::makebool(true,0)); }
    void call_disable() { piw::tsd_fastcall(__disable, this, 0); enqueue_slow(piw::makebool(false,0)); }

    void mod(unsigned long long t, unsigned s);
    void edge_changed(unsigned long long t, unsigned s);
    void clock_ping(unsigned long long t);

    int gc_traverse(void *v, void *a) const { int r; if((r=action_.gc_traverse(v,a))!=0) return r; if((r=thing_t::gc_traverse(v,a))!=0) return r; return 0; }
    int gc_clear() { action_.gc_clear(); piw::thing_t::gc_clear(); return 0; }

    bool enabled_;
    schedimpl_t *scheduler_;
    pic::flipflop_functor_t<piw::change_nb_t> action_;
    unsigned clkcount_;

    std::vector<std::list<edge_constraint_t> > edge_constraints_;
    std::vector<std::list<point_constraint_t> > point_constraints_;
    std::vector<std::list<zone_constraint_t> > zone_constraints_;

    unsigned long modstatus_;
    unsigned long edgestatus_;
    unsigned long zonestatus_;

    bool havemods_;
};

namespace 
{
    struct clocksig_t: virtual pic::counted_t
    {
        clocksig_t(unsigned s, piw::clockinterp_t *ctl): name_(s), controller_(ctl), current_stamp_(0), current_clock_(0)
        {
        }

        void insert_zone(zone_constraint_t *x)
        {
            zone_constraints_.insert(x);
        }

        void remove_zone(zone_constraint_t *x)
        {
            zone_constraints_.erase(x);
        }

        void insert_bound(edge_constraint_t *x)
        {
            bound_constraints_.insert(x);
        }

        void remove_bound(edge_constraint_t *x)
        {
            bound_constraints_.erase(x);
        }

        void insert_mod(mod_constraint_t *x, float divisor, float remainder)
        {
            unsigned t = mod_size((unsigned)divisor);
            pic::msg() << "table size " << t << " for divisor " << divisor << pic::log;

            std::map<unsigned,mod_table_t>::iterator i;

            if((i=mod_tables_.find(t)) == mod_tables_.end())
            {
                std::pair<std::map<unsigned,mod_table_t>::iterator,bool> p = mod_tables_.insert(std::make_pair(t,mod_table_t(t,current_stamp_,current_clock_)));
                i=p.first;
            }

            i->second.add_constraint(x,divisor,remainder);
        }

        void remove_mod(mod_constraint_t *x,float divisor, float remainder)
        {
            unsigned t = mod_size((unsigned)divisor);

            std::map<unsigned,mod_table_t>::iterator i;

            if((i=mod_tables_.find(t)) != mod_tables_.end())
            {
                i->second.remove_constraint(x,divisor,remainder);
            }
        }

        void insert_edge(float v, edge_constraint_t *x)
        {
            edge_table_.insert_edge(v,x);
        }

        void remove_edge(float v, edge_constraint_t *x)
        {
            edge_table_.remove_edge(v,x);
        }

        void clock_full_reset(unsigned long long t)
        {
            float c = controller_->interpolate_clock(name_,t);
            current_stamp_ = t;

            clock_cycled(t,c);

            //pic::msg() << "clock " << name_ << " full reset to " << c << pic::log;
        }

        void clock_cycled(unsigned long long t, float c)
        {
            std::map<unsigned,mod_table_t>::iterator mti;

            edge_table_.reset(t,c);
            for(mti=mod_tables_.begin(); mti!=mod_tables_.end(); mti++)
            {
                mti->second.reset(t,c);
            }

            std::set<zone_constraint_t *>::iterator zi;
            for(zi=zone_constraints_.begin(); zi!=zone_constraints_.end(); zi++)
            {
                (*zi)->update_edge(c);
            }

            std::set<edge_constraint_t *>::iterator ei;
            for(ei=bound_constraints_.begin(); ei!=bound_constraints_.end(); ei++)
            {
                (*ei)->update_edge(c);
            }

            current_stamp_ = t;
            current_clock_ = c;
        }

        void segment_next(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long ubound_time, float ubound_clock)
        {
            edge_table_.sweep(q,ubound_time,ubound_clock);

            std::map<unsigned,mod_table_t>::iterator mti;
            for(mti=mod_tables_.begin(); mti!=mod_tables_.end(); mti++)
            {
                mti->second.sweep(q,ubound_time,ubound_clock);
            }
        }

        void clock_next(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long ubound_time)
        {
            while(current_stamp_<ubound_time)
            {
                unsigned long long reset_time = controller_->interpolate_time(name_,0,current_stamp_);
                float bound = controller_->get_mod(name_);

                if(reset_time<ubound_time && current_clock_>0)
                {
                    segment_next(q,reset_time,bound);
                    clock_cycled(reset_time,0);
                    //pic::msg() << "clock " << name_ << " cycled " << current_clock_ << pic::log;
                }
                else
                {
                    float working_clock = controller_->interpolate_clock(name_,ubound_time);
                    //pic::logmsg() << "working: " << working_clock << " current: " << current_stamp_ << " ubound: " << ubound_time;
                    segment_next(q,ubound_time,working_clock);
                    current_stamp_=ubound_time;
                    current_clock_=working_clock;
                }
            }
        }

        unsigned name_;
        piw::clockinterp_t *controller_;
        edge_table_t edge_table_;
        std::map<unsigned,mod_table_t> mod_tables_;
        std::set<edge_constraint_t *> bound_constraints_;
        std::set<zone_constraint_t *> zone_constraints_;
        unsigned long long current_stamp_;
        float current_clock_;
    };

};

struct piw::scheduler_t::impl_t: piw::wire_t, piw::clockdomain_t, piw::clocksink_t, piw::decode_ctl_t, piw::event_data_sink_t
{
    impl_t(unsigned s): signals_(s), clkcount_(s), upstream_(0), running_(false), previous_(0), interp_(s), decoder_(this),sigmask_(1)
    {
        for(unsigned i=0;i<clkcount_;i++)
        {
            signals_[i] = pic::ref(new clocksig_t(i,&interp_));
            sigmask_ |= (1ULL<<(i+1));
        }

        tsd_clockdomain(this);
        sink(this,"scheduler");
        tick_enable(false);
    }

    piw::wire_t *wire_create(const piw::event_data_source_t &es)
    {
        wire_closed();
        subscribe(es);
        return this;
    } 

    bct_clocksink_t *get_clock() { return this; } 

    float current(unsigned clock)
    {
        if(clock<clkcount_)
        {
            return signals_[clock].ptr()->current_clock_;
        }

        return 0.0;
    }

    void invalidate()
    {
        unsubscribe();
        disconnect();
    }

    ~impl_t()
    {
        invalidate();
    }

    void set_clock(bct_clocksink_t *clock)
    {
        if(upstream_)
            remove_upstream(upstream_);

        upstream_=clock;

        if(upstream_)
            add_upstream(upstream_);
    }

    void set_latency(unsigned latency)
    {
        set_sink_latency(latency);
    }

    void wire_closed()
    {
        invalidate();
    }

    void run_segment(unsigned long long now)
    {
        std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> q;

        for(unsigned i=0;i<clkcount_;i++)
        {
            signals_[i]->clock_next(q,now);
        }

        std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t>::iterator qi;

        while((qi=q.begin())!= q.end())
        {
            clkevent_t e = qi->second;
            q.erase(qi);

            switch(e.type)
            {
                case ET_MOD:
                    e.mod->mod(e.time);
                    break;

                case ET_ZONEGOOD:
                    e.zone->zone_good(e.time);
                    break;

                case ET_ZONEBAD:
                    e.zone->zone_bad(e.time);
                    break;

                case ET_EDGEGOOD:
                    e.event->edge_good(e.time);
                    break;

                case ET_EDGEBAD:
                    e.event->edge_bad(e.time);
                    break;
            }
        }
    }

    void clock_full_reset(unsigned long long now)
    {
        for(unsigned i=0;i<clkcount_;i++)
        {
            signals_[i]->clock_full_reset(now);
        }
    }

    void update_transport(const piw::data_nb_t &d)
    {
        bool ts = d.as_norm()!=0.f;
        unsigned long long tt = d.time();

        if(ts == running_)
        {
            return;
        }

        if(running_)
        {
            run_segment(tt);
            previous_=tt;
        }
        else
        {
            previous_=tt;
        }

        running_=ts;

        if(ts)
        {
            clock_full_reset(tt);
            pic::msg() << "full clock reset" << pic::log;
        }

        pic::msg() << "transport is " << (ts?"running":"stopped") << pic::log;
    }

    void clocksink_ticked(unsigned long long from,unsigned long long now)
    {
        //pic::logmsg() << "sched tick " << from << "-" << now;
        unsigned sig;
        piw::data_nb_t d;

        if(iterator_.isvalid())
        {
            while(iterator_->next(sigmask_,sig,d,now))
            {
                switch(sig)
                {
                    case 1:
                        //pic::logmsg() << "transport " << d << " at " << d.time();
                        update_transport(d);
                        break;

                    default:
                        //pic::logmsg() << "clk " << (sig-2) <<  " data "  << d << " at " << d.time();
                        interp_.recv_clock(sig-2,d);
                        break;
                }
            }
        }

        if(previous_ < now)
        {
            if(running_)
            {
                run_segment(now+1);
            }

            previous_=now+1;
        }
    }

    bool isrunning()
    {
        return running_;
    }

    void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
    {
        pic::logmsg() << "scheduler start";
        iterator_=b.iterator();

        piw::data_nb_t d;
        unsigned long long t=id.time();

        iterator_->reset(1,t);

        for(unsigned i=0; i<clkcount_; ++i)
        {
            iterator_->reset(i+2,t);
        }

        tick_suppress(false);
    }

    bool event_end(unsigned long long t)
    {
        pic::logmsg() << "scheduler end"; 
        update_transport(piw::makefloat_bounded_nb(1,0,0,0,t));
        tick_suppress(true);
        iterator_.clear();
        return true;
    }

    void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
    {
        iterator_->set_signal(sig,n);
        iterator_->reset(sig,t);
    }

    std::vector<pic::ref_t<clocksig_t> > signals_;
    unsigned clkcount_;
    bct_clocksink_t *upstream_;
    bool running_;
    unsigned long long previous_;
    piw::clockinterp_t interp_;
    piw::decoder_t decoder_;
    unsigned long sigmask_;
    piw::xevent_data_buffer_t::iter_t iterator_;
};

void piw::event_t::impl_t::clear()
{
    bool e = disable();

    for(unsigned i=0; i<clkcount_; i++)
    {
        edge_constraints_[i].clear();
        point_constraints_[i].clear();
        zone_constraints_[i].clear();
    }

    reenable(e);
}

void piw::event_t::impl_t::lower(unsigned s,float v)
{
    bool e = disable();
    edge_constraints_[s].push_back(edge_constraint_t(this,s,true,v));
    reenable(e);
}

void piw::event_t::impl_t::zone(unsigned s,unsigned d, float r1, float r2)
{
    bool e = disable();
    if (r1<0) r1=0;
    if (r2<0) r2=0;
    zone_constraints_[s].push_back(zone_constraint_t(this,s,d,r1,r2));
    reenable(e);
}

void piw::event_t::impl_t::modulo(unsigned s,unsigned d, float r)
{
    bool e = disable();
    if (r<0) r=0;
    point_constraints_[s].push_back(point_constraint_t(this,s,d,r));
    reenable(e);
}

void piw::event_t::impl_t::upper(unsigned s,float v)
{
    bool e = disable();
    edge_constraints_[s].push_back(edge_constraint_t(this,s,false,v));
    reenable(e);
}

bool piw::event_t::impl_t::disable()
{
    if(!enabled_) return false;

    enabled_=false;

    for(unsigned i=0; i<clkcount_; i++)
    {
        clocksig_t *c = scheduler_->signals_[i].ptr();

        std::list<edge_constraint_t> &lc = edge_constraints_[i];
        std::list<point_constraint_t> &mc = point_constraints_[i];
        std::list<zone_constraint_t> &zc = zone_constraints_[i];

        std::list<edge_constraint_t>::iterator li;
        std::list<point_constraint_t>::iterator mi;
        std::list<zone_constraint_t>::iterator zi;

        for(li=lc.begin(); li!=lc.end(); li++)
        {
            c->remove_edge(li->edge_value_,&(*li));
            c->remove_bound(&(*li));
        }

        for(mi=mc.begin(); mi!=mc.end(); mi++)
        {
            c->remove_mod(&(*mi), mi->mod_divisor, mi->mod_remainder);
        }

        for(zi=zc.begin(); zi!=zc.end(); zi++)
        {
            c->remove_mod(&(*zi), zi->mod_divisor, zi->mod_remainder1);
            c->remove_mod(&(*zi), zi->mod_divisor, zi->mod_remainder2);
            c->remove_zone(&(*zi));
        }
    }

    return true;
}

void piw::event_t::impl_t::enable()
{
    if(enabled_) return;

    enabled_=true;
    modstatus_=0;
    edgestatus_=0;
    zonestatus_=0;
    havemods_ = false;

    for(unsigned i=0; i<clkcount_; i++)
    {
        clocksig_t *c = scheduler_->signals_[i].ptr();
        float v = c->current_clock_;

        std::list<edge_constraint_t> &lc = edge_constraints_[i];
        std::list<point_constraint_t> &mc = point_constraints_[i];
        std::list<zone_constraint_t> &zc = zone_constraints_[i];

        std::list<edge_constraint_t>::iterator li;
        std::list<point_constraint_t>::iterator mi;
        std::list<zone_constraint_t>::iterator zi;

        bool eok = true;
        bool zok = true;

        for(li=lc.begin(); li!=lc.end(); li++)
        {
            li->update_edge(v);
            c->insert_edge(li->edge_value_,&(*li));
            c->insert_bound(&(*li));
            if(!li->applies()) eok=false;
        }

        for(zi=zc.begin(); zi!=zc.end(); zi++)
        {
            zi->update_edge(v);
            c->insert_mod(&(*zi), zi->mod_divisor, zi->mod_remainder1);
            c->insert_mod(&(*zi), zi->mod_divisor, zi->mod_remainder2);
            c->insert_zone(&(*zi));
            if(!zi->applies()) zok=false;
        }

        for(mi=mc.begin(); mi!=mc.end(); mi++)
        {
            c->insert_mod(&(*mi), mi->mod_divisor, mi->mod_remainder);
            havemods_ = true;
        }

        if(!eok)
        {
            edgestatus_ |= (1<<i);
        }

        if(!zok)
        {
            zonestatus_ |= (1<<i);
        }
    }
}

void piw::event_t::impl_t::clock_ping(unsigned long long t)
{
    pic::msg() << (void *)this << " event pinged! " << t << pic::log;

    try
    {
        action_(piw::makefloat_bounded_nb(1,0,0,1,t));
    }
    CATCHLOG()

    try
    {
        action_(piw::makefloat_bounded_nb(1,0,0,0,t+1));
    }
    CATCHLOG()
}

void piw::event_t::impl_t::mod(unsigned long long t, unsigned s)
{
    if(scheduler_->isrunning() && !edgestatus_ && !zonestatus_)
    {
        clock_ping(t);
    }
}

void piw::event_t::impl_t::edge_changed(unsigned long long t, unsigned s)
{
    std::list<edge_constraint_t> &lc = edge_constraints_[s];
    std::list<edge_constraint_t>::iterator li;
    std::list<zone_constraint_t> &zc = zone_constraints_[s];
    std::list<zone_constraint_t>::iterator zi;

    bool eok = true;
    bool zok = true;

    for(zi=zc.begin(); zi!=zc.end(); zi++)
    {
        if(!zi->applies())
        {
            zok=false;
        }
    }

    for(li=lc.begin(); li!=lc.end(); li++)
    {
        if(!li->applies())
        {
            //pic::msg() << "edge " << (void *)(&(*li)) << " v=" << li->edge_value_ << " is bad" << pic::log;
            eok=false;
        }
    }

    if(zok)
    {
        zonestatus_ &= ~(1<<s);
    }
    else
    {
        zonestatus_ |= (1<<s);
    }

    if(eok)
    {
        edgestatus_ &= ~(1<<s);
    }
    else
    {
        edgestatus_ |= (1<<s);
    }

    //pic::msg() << "edge changed: s=" << s << " eok=" << eok << " zok=" << zok << " es=" << edgestatus_ << " zs=" << zonestatus_ << " hm=" << havemods_ << pic::log;

    if(scheduler_->isrunning() && !havemods_ && !edgestatus_ && !zonestatus_)
    {
        clock_ping(t);
    }
}

void point_constraint_t::mod(unsigned long long t)
{
    event_->mod(t,signal_);
}

void zone_constraint_t::event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value)
{
    if(event_value==mod_remainder1)
    {
        q.insert(std::make_pair(std::make_pair(5,event_time),clkevent_t(ET_ZONEGOOD,signal_,this,event_time)));
    }
    else
    {
        q.insert(std::make_pair(std::make_pair(3,event_time),clkevent_t(ET_ZONEBAD,signal_,this,event_time)));
    }
}

void point_constraint_t::event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value)
{
    q.insert(std::make_pair(std::make_pair(10,event_time),clkevent_t(ET_MOD,signal_,this,event_time)));
}

void edge_constraint_t::event(std::multimap<std::pair<unsigned char,unsigned long long>, clkevent_t> &q, unsigned long long event_time, float event_value)
{
    if(edge_lower_)
    {
        q.insert(std::make_pair(std::make_pair(5,event_time),clkevent_t(ET_EDGEGOOD,signal_,this,event_value,event_time)));
    }
    else
    {
        q.insert(std::make_pair(std::make_pair(3,event_time),clkevent_t(ET_EDGEBAD,signal_,this,event_value,event_time)));
    }
}

void zone_constraint_t::zone_good(unsigned long long t)
{
    zone_true_ = true;
    event_->edge_changed(t,signal_);
}

void zone_constraint_t::zone_bad(unsigned long long t)
{
    zone_true_ = false;
    event_->edge_changed(t,signal_);
}

void edge_constraint_t::edge_good(unsigned long long t)
{
    //pic::msg() << "edge good " << (void *)this << " v=" << edge_value_ << pic::log;
    edge_true_ = true;
    event_->edge_changed(t,signal_);
}

void edge_constraint_t::edge_bad(unsigned long long t)
{
    edge_true_ = false;
    event_->edge_changed(t,signal_);
}

void piw::event_t::event_clear() { impl_->call_clear(); }
void piw::event_t::lower_bound(unsigned signal, float value) { impl_->call_lower(signal-1, value); }
void piw::event_t::upper_bound(unsigned signal, float value) { impl_->call_upper(signal-1, value); }
void piw::event_t::zone(unsigned signal, unsigned divisor, float r1, float r2) { impl_->call_zone(signal-1, divisor,r1,r2); }
void piw::event_t::modulo(unsigned signal, unsigned divisor, float remainder) { impl_->call_modulo(signal-1, divisor, remainder); }
void piw::event_t::event_enable() { impl_->call_enable(); }
void piw::event_t::event_disable() { impl_->call_disable(); }
int piw::event_t::gc_traverse(void *v, void *a) const { return impl_->gc_traverse(v,a); }
int piw::event_t::gc_clear() { return impl_->gc_clear(); }
piw::event_t::event_t(scheduler_t *sched,bool e,const piw::change_t &ec): controlled_t(e), impl_(new impl_t(sender(),sched->impl_,sched->impl_->clkcount_,ec)) { } 
piw::event_t::~event_t() { delete impl_; } 
piw::scheduler_t::scheduler_t(unsigned signals): impl_(new impl_t(signals)) { } 
piw::scheduler_t::~scheduler_t() { delete impl_; } 


float piw::scheduler_t::current(unsigned clock) { return impl_->current(clock-1); }
piw::cookie_t piw::scheduler_t::cookie() { return impl_->decoder_.cookie(); }
