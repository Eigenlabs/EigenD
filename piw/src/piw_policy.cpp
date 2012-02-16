
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

#include <piw/piw_policy.h>

namespace
{
    struct resampling_converter_t: piw::converter_t
    {
        resampling_converter_t(): clean_(true)
        {
        }

        piw::dataqueue_t convert(const piw::dataqueue_t &q,unsigned long long) { if(clean_) return q; else return piw::dataqueue_t(); }
        void clock_status(bool status) { clean_=status; }
        int ticked(unsigned long long from,unsigned long long t, unsigned long sr, unsigned bs) { return TICK_DISABLE; }

        bool clean_;
    };

    struct gt_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        gt_filter_t(unsigned s): s_(s) {}

        gt_filter_t(const gt_filter_t &o): s_(o.s_) {}
        bool operator==(const gt_filter_t &o) const { return s_==o.s_; }
        gt_filter_t &operator=(const gt_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_pathlen()==0 || x.as_path()[x.as_pathlen()-1]<=s_)
            {
                return piw::makenull_nb(x.time());
            }

            return x;
        }
    };

    struct lt_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        lt_filter_t(unsigned s): s_(s) {}

        lt_filter_t(const lt_filter_t &o): s_(o.s_) {}
        bool operator==(const lt_filter_t &o) const { return s_==o.s_; }
        lt_filter_t &operator=(const lt_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_pathlen()==0 || x.as_path()[x.as_pathlen()-1]>=s_)
            {
                return piw::makenull_nb(x.time());
            }

            return x;
        }
    };

    struct last_filter_t: virtual public pic::lckobject_t
    {
        last_filter_t() {}

        last_filter_t(const last_filter_t &o) {}
        bool operator==(const last_filter_t &o) const { return true; }
        last_filter_t &operator=(const last_filter_t &o) { return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_pathlen()==0)
            {
                return piw::makenull_nb(x.time());
            }

            return piw::pathone_nb(x.as_path()[x.as_pathlen()-1],x.time());
        }
    };

    struct event_filter_t: virtual public pic::lckobject_t
    {
        event_filter_t() {}

        event_filter_t(const event_filter_t &o) {}
        bool operator==(const event_filter_t &o) const { return true; }
        event_filter_t &operator=(const event_filter_t &o) { return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_pathlen()==0)
            {
                return piw::makenull_nb(x.time());
            }

            return piw::pathone_nb(x.as_path()[x.as_pathlen()-1],x.time());
        }
    };

    struct eventchannel_aggregation_filter_t: virtual public pic::lckobject_t
    {
        unsigned g_,c_;

        eventchannel_aggregation_filter_t(unsigned g,unsigned c): g_(g),c_(c) {}

        eventchannel_aggregation_filter_t(const eventchannel_aggregation_filter_t &o): g_(o.g_),c_(o.c_) {}
        bool operator==(const eventchannel_aggregation_filter_t &o) const { return g_==o.g_ && c_==o.c_; }
        eventchannel_aggregation_filter_t &operator=(const eventchannel_aggregation_filter_t &o) { g_=o.g_; c_=o.c_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path())
            {
                return x;
            }

            piw::data_nb_t d= pathprepend_nb(pathprepend_event_nb(x,g_),c_);
            return d;
        }
    };

    struct event_aggregation_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        event_aggregation_filter_t(unsigned s): s_(s) {}

        event_aggregation_filter_t(const event_aggregation_filter_t &o): s_(o.s_) {}
        bool operator==(const event_aggregation_filter_t &o) const { return s_==o.s_; }
        event_aggregation_filter_t &operator=(const event_aggregation_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path())
            {
                return x;
            }

            piw::data_nb_t d= pathprepend_event_nb(x,s_);
            return d;
        }
    };

    struct event_deaggregation_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        event_deaggregation_filter_t(unsigned s): s_(s) {}

        event_deaggregation_filter_t(const event_deaggregation_filter_t &o): s_(o.s_) {}
        bool operator==(const event_deaggregation_filter_t &o) const { return s_==o.s_; }
        event_deaggregation_filter_t &operator=(const event_deaggregation_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_patheventlen()<1 || x.as_pathevent()[0]!=s_)
            {
                return piw::makenull_nb(x.time());
            }

            piw::data_nb_t d = patheventpretruncate_nb(x);
            return d;
        }
    };

    struct event_deaggregation_filter2_t: virtual public pic::lckobject_t
    {
        unsigned s1_;
        unsigned s2_;

        event_deaggregation_filter2_t(unsigned s1,unsigned s2): s1_(s1),s2_(s2) {}

        event_deaggregation_filter2_t(const event_deaggregation_filter2_t &o): s1_(o.s1_),s2_(o.s2_) {}
        bool operator==(const event_deaggregation_filter2_t &o) const { return s1_==o.s1_ && s2_==o.s2_; }
        event_deaggregation_filter2_t &operator=(const event_deaggregation_filter2_t &o) { s1_=o.s1_; s2_=o.s2_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_patheventlen()<1)
            {
                return piw::makenull_nb(x.time());
            }

            if(x.as_pathevent()[0]!=s1_ && x.as_pathevent()[0]!=s2_)
            {
                return piw::makenull_nb(x.time());
            }

            piw::data_nb_t d = patheventpretruncate_nb(x);
            return d;
        }
    };

    struct deaggregation_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        deaggregation_filter_t(unsigned s): s_(s) {}

        deaggregation_filter_t(const deaggregation_filter_t &o): s_(o.s_) {}
        bool operator==(const deaggregation_filter_t &o) const { return s_==o.s_; }
        deaggregation_filter_t &operator=(const deaggregation_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path() || x.as_pathchannellen()<1 || x.as_path()[0]!=s_)
            {
                return piw::makenull_nb(x.time());
            }

            piw::data_nb_t d = pathpretruncate_nb(x);
            return d;
        }
    };

    struct aggregation_filter3_t: virtual public pic::lckobject_t
    {
        unsigned s1_,s2_,s3_;

        aggregation_filter3_t(unsigned s1,unsigned s2,unsigned s3): s1_(s1), s2_(s2), s3_(s3) {}

        aggregation_filter3_t(const aggregation_filter3_t &o): s1_(o.s1_), s2_(o.s2_), s3_(o.s3_) {}
        bool operator==(const aggregation_filter3_t &o) const { return s1_==o.s1_ && s2_==o.s2_ && s3_==o.s3_; }
        aggregation_filter3_t &operator=(const aggregation_filter3_t &o) { s1_=o.s1_; s2_=o.s2_; s3_=o.s3_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path())
            {
                return x;
            }

            piw::data_nb_t d(x);
            
            if(s3_) d=pathprepend_nb(d,s3_);
            if(s2_) d=pathprepend_nb(d,s2_);
            if(s1_) d=pathprepend_nb(d,s1_);

            return d;
        }
    };

    struct aggregation_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;

        aggregation_filter_t(unsigned s): s_(s) {}

        aggregation_filter_t(const aggregation_filter_t &o): s_(o.s_) {}
        bool operator==(const aggregation_filter_t &o) const { return s_==o.s_; }
        aggregation_filter_t &operator=(const aggregation_filter_t &o) { s_=o.s_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path())
            {
                return x;
            }

            piw::data_nb_t d = pathprepend_nb(x,s_);
            return d;
        }
    };

    struct signal_cnc_filter_t: virtual public pic::lckobject_t
    {
        unsigned s_;
        unsigned g_;

        signal_cnc_filter_t(unsigned s, unsigned g): s_(s), g_(g) {}

        signal_cnc_filter_t(const signal_cnc_filter_t &o): s_(o.s_), g_(o.g_) {}
        bool operator==(const signal_cnc_filter_t &o) const { return s_==o.s_ && g_==o.g_; }
        signal_cnc_filter_t &operator=(const signal_cnc_filter_t &o) { s_=o.s_; g_=o.g_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(!x.is_path())
            {
                return x;
            }

            piw::data_nb_t d = x;

            if(g_)
            {
                d=piw::pathone_nb(g_,x.time());
            }

            if((s_&0xff)!=0)
            {
                d=pathprepend_nb(d,(s_&0xff));
            }

            if((s_&0xff00)!=0)
            {
                d=pathprepend_nb(d,(s_&0xff00)>>8);
            }

            return d;
        }
    };

    struct signal_dsc_filter_t: virtual public pic::lckobject_t
    {
        piw::data_nb_t p_;
        unsigned s_;
        unsigned g_;

        signal_dsc_filter_t(unsigned s,unsigned g,const piw::data_nb_t &p): p_(p), s_(s), g_(g) {}

        signal_dsc_filter_t(const signal_dsc_filter_t &o): p_(o.p_),s_(o.s_),g_(o.g_) {}
        bool operator==(const signal_dsc_filter_t &o) const { return p_==o.p_ && s_==o.s_ && g_==o.g_; }
        signal_dsc_filter_t &operator=(const signal_dsc_filter_t &o) { p_=o.p_; s_=o.s_; g_=o.g_; return *this; }

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            if(x.compare_path_beginning(p_)==0)
            {
                piw::data_nb_t d = x;
                
                d=piw::pathpretruncate_nb(d,p_.as_pathlen());
                
                if(g_)
                {
                    d=piw::pathprepend_event_nb(d,g_);
                }

                if((s_&0xff)!=0)
                {
                    d=pathprepend_nb(d,(s_&0xff));
                }

                if((s_&0xff00)!=0)
                {
                    d=pathprepend_nb(d,(s_&0xff00)>>8);
                }

                return d;
            }
            
            return piw::makenull_nb(x.time());
        }
    };
}

piw::d2d_nb_t piw::last_gt_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(gt_filter_t(s));
}

piw::d2d_nb_t piw::last_lt_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(lt_filter_t(s));
}

piw::d2d_nb_t piw::event_filter()
{
    return piw::d2d_nb_t::callable(event_filter_t());
}

piw::d2d_nb_t piw::last_filter()
{
    return piw::d2d_nb_t::callable(last_filter_t());
}

piw::d2d_nb_t piw::eventchannel_aggregation_filter(unsigned g,unsigned c)
{
    return piw::d2d_nb_t::callable(eventchannel_aggregation_filter_t(g,c));
}

piw::d2d_nb_t piw::event_aggregation_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(event_aggregation_filter_t(s));
}

piw::d2d_nb_t piw::aggregation_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(aggregation_filter_t(s));
}

piw::d2d_nb_t piw::aggregation_filter3(unsigned s1, unsigned s2, unsigned s3)
{
    return piw::d2d_nb_t::callable(aggregation_filter3_t(s1,s2,s3));
}

piw::d2d_nb_t piw::deaggregation_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(deaggregation_filter_t(s));
}

piw::d2d_nb_t piw::event_deaggregation_filter(unsigned s)
{
    return piw::d2d_nb_t::callable(event_deaggregation_filter_t(s));
}

piw::d2d_nb_t piw::event_deaggregation_filter2(unsigned s1,unsigned s2)
{
    return piw::d2d_nb_t::callable(event_deaggregation_filter2_t(s1,s2));
}

piw::d2d_nb_t piw::signal_cnc_filter(unsigned s, unsigned g)
{
    return piw::d2d_nb_t::callable(signal_cnc_filter_t(s,g));
}

piw::d2d_nb_t piw::signal_dsc_filter(unsigned s, unsigned g, const char *p)
{
    return piw::d2d_nb_t::callable(signal_dsc_filter_t(s,g,piw::parsepath_nb(p)));
}

static piw::data_nb_t root_filter__(const piw::data_nb_t &d)
{
    return piw::pathnull_nb(d.time());
}

static piw::data_nb_t null_filter__(const piw::data_nb_t &d)
{
    return d;
}

piw::d2d_nb_t piw::null_filter()
{
    return piw::d2d_nb_t::callable(null_filter__);
}

piw::d2d_nb_t piw::root_filter()
{
    return piw::d2d_nb_t::callable(root_filter__);
}

piw::converter_ref_t piw::null_converter()
{
    return converter_ref_t();
}

piw::converter_ref_t piw::resampling_converter()
{
    return pic::ref(new resampling_converter_t);
}

