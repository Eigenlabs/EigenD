
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

#include <piw/piw_normalize.h>
#include <piw/piw_tsd.h>
#include <sstream>

struct bint_norm_t
{
    float min_,max_,rest_;
    bint_norm_t(int min,int max,int rest): min_(min),max_(max),rest_(rest) {}
    bint_norm_t(const bint_norm_t &o): min_(o.min_), max_(o.max_), rest_(o.rest_) {}
    bool operator==(const bint_norm_t &o) const { return min_==o.min_ && max_==o.max_ && rest_==o.rest_; }
    bint_norm_t &operator=(const bint_norm_t &o) { min_=o.min_; max_=o.max_; rest_=o.rest_; return *this; }

    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        if(!x.is_long()) return piw::makefloat_bounded_nb(max_,min_,rest_,rest_,x.time());
        return piw::makefloat_bounded_nb(max_,min_,rest_,x.as_long(),x.time());
    }
};

struct bint_denorm_t
{
    float min_,max_,rest_;
    bint_denorm_t(int min,int max,int rest): min_(min),max_(max),rest_(rest) { }
    bint_denorm_t(const bint_denorm_t &o): min_(o.min_), max_(o.max_), rest_(o.rest_) {}
    bool operator==(const bint_denorm_t &o) const { return min_==o.min_ && max_==o.max_ && rest_==o.rest_; }
    bint_denorm_t &operator=(const bint_denorm_t &o) { min_=o.min_; max_=o.max_; rest_=o.rest_; return *this; }

    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        return piw::makelong_nb((long)(x.as_renorm(min_,max_,rest_)),x.time());
    }
};

piw::d2d_nb_t piw::bint_normalizer(int min, int max, int rest) { return piw::d2d_nb_t::callable(bint_norm_t(min,max,rest)); }
piw::d2d_nb_t piw::bint_denormalizer(int min, int max, int rest) { return piw::d2d_nb_t::callable(bint_denorm_t(min,max,rest)); }

struct bfloat_norm_t : pic::sink_t<piw::data_nb_t(const piw::data_nb_t &)>
{
    float min_,max_,rest_;
    bfloat_norm_t(float min,float max,float rest):min_(min),max_(max),rest_(rest) {}
    bfloat_norm_t(const bfloat_norm_t &o):min_(o.min_), max_(o.max_), rest_(o.rest_) {}
    bool operator==(const bfloat_norm_t &o) const { return min_==o.min_ && max_==o.max_ && rest_==o.rest_; }
    bfloat_norm_t &operator=(const bfloat_norm_t &o) { min_=o.min_; max_=o.max_; rest_=o.rest_; return *this; } 
    bool iscallable() const { return true; }

    piw::data_nb_t invoke(const piw::data_nb_t &x) const
    {
        if(!x.is_float()) return piw::makefloat_bounded_nb(max_,min_,rest_,rest_,x.time());
        return piw::makefloat_bounded_nb(max_,min_,rest_,x.as_float(),x.time());
    }
};

struct bfloat_denorm_t : pic::sink_t<piw::data_nb_t(const piw::data_nb_t &)>
{
    float min_,max_,rest_;
    bfloat_denorm_t(float min,float max,float rest):min_(min),max_(max),rest_(rest) { }
    bfloat_denorm_t(const bfloat_denorm_t &o):min_(o.min_), max_(o.max_), rest_(o.rest_) {}
    bool operator==(const bfloat_denorm_t &o) const { return min_==o.min_ && max_==o.max_ && rest_==o.rest_; }
    bfloat_denorm_t &operator=(const bfloat_denorm_t &o) { min_=o.min_; max_=o.max_; rest_=o.rest_; return *this; } 
    bool iscallable() const { return true; }

    piw::data_nb_t invoke(const piw::data_nb_t &x) const
    {
        return piw::makefloat_nb(x.as_renorm(min_,max_,rest_),x.time());
    }
};

piw::d2d_nb_t piw::bfloat_normalizer(float min, float max, float rest) { return piw::d2d_nb_t(pic::ref(new bfloat_norm_t(min,max,rest))); }
piw::d2d_nb_t piw::bfloat_denormalizer(float min, float max, float rest) { return piw::d2d_nb_t(pic::ref(new bfloat_denorm_t(min,max,rest))); }

struct string_norm_t
{
    bool operator==(const string_norm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        if(!x.is_string()) return piw::makefloat_bounded_nb(256,0,0,0,x.time());
        unsigned l = x.as_stringlen();
        float xv = 0;

        if(l>0)
        {
            xv=(float)x.as_string()[0];
        }

        return piw::makefloat_bounded_nb(256,0,0,xv,x.time());
    }
};

struct string_denorm_t
{
    bool operator==(const string_denorm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        std::stringstream oss; oss << x.as_norm();
        return piw::makestring_nb(oss.str());
    }
};

piw::d2d_nb_t piw::string_normalizer() { return piw::d2d_nb_t::callable(string_norm_t()); }
piw::d2d_nb_t piw::string_denormalizer() { return piw::d2d_nb_t::callable(string_denorm_t()); }

struct bool_norm_t
{
    bool operator==(const bool_norm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        if(!x.is_bool() || !x.as_bool()) return piw::makefloat_bounded_nb(1,0,0,0,x.time());
        return piw::makefloat_bounded_nb(1,0,0,1,x.time());
    }
};

struct bool_denorm_t
{
    bool operator==(const bool_denorm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        return piw::makebool_nb(x.as_norm()>=0.0,x.time());
    }
};

piw::d2d_nb_t piw::bool_normalizer() { return piw::d2d_nb_t::callable(bool_norm_t()); }
piw::d2d_nb_t piw::bool_denormalizer() { return piw::d2d_nb_t::callable(bool_denorm_t()); }

struct convert_t: piw::change_nb_t::sinktype_t
{
    piw::d2d_nb_t a_;
    piw::change_nb_t s_;

    convert_t(const piw::d2d_nb_t &a,const piw::change_nb_t &s): a_(a), s_(s) {}
    convert_t(const convert_t &o): a_(o.a_), s_(o.s_) {}
    void invoke(const piw::data_nb_t &x) const { piw::data_nb_t y=a_(x); s_(y); }
    bool iscallable() const { return true; }

    int gc_visit(void *v, void *a) const
    {
        int r;
        if((r=a_.gc_traverse(v,a))!=0) return r;
        if((r=s_.gc_traverse(v,a))!=0) return r;
        return 0;
    }
};

piw::change_nb_t piw::d2d_convert(const d2d_nb_t &a, const change_nb_t &s) { return piw::change_nb_t(pic::ref(new convert_t(a,s))); }

struct chain_t: piw::change_nb_t::sinktype_t
{
    piw::d2d_nb_t a_,b_;
    piw::change_nb_t s_;

    chain_t(const piw::d2d_nb_t &a,const piw::d2d_nb_t &b, const piw::change_nb_t &s): a_(a), b_(b),s_(s) {}
    void invoke(const piw::data_nb_t &x) const { piw::data_nb_t y=b_(a_(x)); s_(y); }
    bool iscallable() const { return true; }

    int gc_visit(void *v, void *a) const
    {
        int r;
        if((r=a_.gc_traverse(v,a))!=0) return r;
        if((r=b_.gc_traverse(v,a))!=0) return r;
        if((r=s_.gc_traverse(v,a))!=0) return r;
        return 0;
    }
};

piw::change_nb_t piw::d2d_chain(const d2d_nb_t &a, const d2d_nb_t &b, const change_nb_t &s) { return piw::change_nb_t(pic::ref(new chain_t(a,b,s))); }

struct null_norm_t
{
    bool operator==(const null_norm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        return piw::makefloat_bounded_nb(1,-1,-1,-1,x.time());
    }
};

struct null_denorm_t
{
    bool operator==(const null_denorm_t &o) const { return true; }
    piw::data_nb_t operator()(const piw::data_nb_t &x) const
    {
        return piw::makenull_nb(x.time());
    }
};

piw::d2d_nb_t piw::null_normalizer() { return piw::d2d_nb_t::callable(null_norm_t()); }
piw::d2d_nb_t piw::null_denormalizer() { return piw::d2d_nb_t::callable(null_denorm_t()); }
