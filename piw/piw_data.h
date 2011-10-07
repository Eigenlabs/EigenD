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

#ifndef __PIW_DATA__
#define __PIW_DATA__

#include "piw_exports.h"
#include <pibelcanto/plugin.h>

#include <picross/pic_error.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_functor.h>
#include <picross/pic_flipflop.h>
#include <picross/pic_atomic.h>
#include <picross/pic_log.h>
#include <piembedded/pie_iostream.h>
#include <piembedded/pie_print.h>

#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

namespace piw
{
    inline void data_atomicity_assertion(bct_data_t d)
    {
#ifdef DEBUG_DATA_ATOMICITY
        if(d && d->tid && !pic_threadid_equal(d->tid, pic_current_threadid()))
        {
            std::stringstream oss;
            oss << "piw_data thread mismatch ";
            oss << d->tid;
            oss << "(data thread) != ";
            oss << pic_current_threadid();
            oss << "(current thread) [";
            pie_print(bct_data_wirelen(d), bct_data_wiredata(d), pie::ostreamwriter, &oss);
            oss << "]";
            PIC_THROW(oss.str().c_str());
        }
#endif
    }

    inline void piw_data_incref_fast(bct_data_t d)
    {
        if(d)
        {
            data_atomicity_assertion(d);

            ++d->count;
        }
    }

    inline void piw_data_decref_fast(bct_data_t d)
    {
        if(d)
        {
            data_atomicity_assertion(d);

            if(--d->count==0)
            {
                bct_data_free(d);
            }
        }
    }

    inline void piw_data_incref_atomic(bct_data_t d) { if(d) pic_atomicinc(&d->count); }
    inline void piw_data_decref_atomic(bct_data_t d) { if(d) if(pic_atomicdec(&d->count)==0) bct_data_free(d); }

    inline float denormalise(float ubound, float lbound, float rest, float norm)
    {
        PIC_ASSERT(lbound<=rest && rest<=ubound);
        float r = (norm>=0) ? ubound-rest : rest-lbound;
        return rest+norm*r;
    }

    inline float normalise(float ubound, float lbound, float rest, float denorm)
    {
        PIC_ASSERT(lbound<=rest && rest<=ubound);
        if(denorm<lbound) denorm=lbound;
        if(denorm>ubound) denorm=ubound;
        float r = (denorm>=rest) ? ubound-rest : rest-lbound;
        return (r==0) ? 0 : (denorm-rest)/r;
    }

    inline double denormalised(double ubound, double lbound, double rest, double norm)
    {
        PIC_ASSERT(lbound<=rest && rest<=ubound);
        double r = (norm>=0) ? ubound-rest : rest-lbound;
        return rest+norm*r;
    }

    inline double normalised(double ubound, double lbound, double rest, double denorm)
    {
        PIC_ASSERT(lbound<=rest && rest<=ubound);
        if(denorm<lbound) denorm=lbound;
        if(denorm>ubound) denorm=ubound;
        double r = (denorm>=rest) ? ubound-rest : rest-lbound;
        return (r==0) ? 0 : (denorm-rest)/r;
    }

    inline double clipd(double ubound, double lbound, double value)
    {
        return std::max(lbound,std::min(ubound,value));
    }

    inline float clip(float ubound, float lbound, float value)
    {
        return std::max(lbound,std::min(ubound,value));
    }

    class data_t;
    class data_nb_t;
    class dataholder_nb_t;

    class PIW_DECLSPEC_CLASS data_base_t
    {
        public:
            virtual ~data_base_t() {}

            virtual void clear() = 0;

            virtual bct_data_t give_copy(unsigned nb) const;

            virtual bct_data_t lend() const = 0;
            virtual bct_data_t give() const = 0;
            virtual bct_data_t give_copy() const = 0;

            virtual data_nb_t make_nb() const = 0;
            virtual data_t make_normal() const = 0;

            unsigned data_type() const { return type(); }
            unsigned type() const { return bct_data_type(_rep); }
            unsigned units() const { return bct_data_units(_rep); }
            unsigned long long time() const { return bct_data_time(_rep); }
            unsigned long count() const { return bct_data_count(_rep); }

            int compare_grist(const data_base_t &d) const;
            int compare(const data_base_t &d, bool ts=true) const;
            int compare_path(const data_base_t &d) const;
            int compare_path_beginning(const data_base_t &d) const;
            int compare_path_lexicographic(const data_base_t &d) const;

            bool operator<(const data_base_t &d) const { return compare(d)<0; }
            bool operator<=(const data_base_t &d) const { return compare(d)<=0; }
            bool operator>(const data_base_t &d) const { return compare(d)>0; }
            bool operator>=(const data_base_t &d) const { return compare(d)>=0; }
            bool operator==(const data_base_t &d) const { return compare(d)==0; }
            bool operator!=(const data_base_t &d) const { return compare(d)!=0; }

            unsigned host_length() const { return bct_data_len(_rep); }
            const void *host_data() const { return bct_data_data(_rep); }
            unsigned wire_length() const { return bct_data_wirelen(_rep); }
            const void *wire_data() const { return bct_data_wiredata(_rep); }

            bool is_null() const { return type()==BCTVTYPE_NULL; }
            bool is_long() const { return type()==BCTVTYPE_INT; }
            bool is_float() const { return type()==BCTVTYPE_FLOAT; }
            bool is_double() const { return type()==BCTVTYPE_DOUBLE; }
            bool is_string() const { return type()==BCTVTYPE_STRING; }
            bool is_path() const { return type()==BCTVTYPE_PATH; }
            bool is_bool() const { return type()==BCTVTYPE_BOOL; }
            bool is_blob() const { return type()==BCTVTYPE_BLOB; }
            bool is_dict() const { return type()==BCTVTYPE_DICT; }
            bool is_array() const { return !is_null(); }

            float as_float() const { PIC_ASSERT(is_float()); return *(float *)host_data(); }
            float as_double() const { PIC_ASSERT(is_double()); return *(double *)host_data(); }

            const float *as_array() const { return bct_data_vector(_rep); }
            unsigned as_arraylen() const { return bct_data_veclen(_rep); }
            float as_array_ubound() const { return bct_data_ubound(_rep); }
            float as_array_lbound() const { return bct_data_lbound(_rep); }
            float as_array_rest() const { return bct_data_rest(_rep); }
            float as_array_member(unsigned i) const { PIC_ASSERT(i<as_arraylen()); return as_array()[i]; }

            float as_norm() const { unsigned l = as_arraylen(); return (l>0)?(*(as_array()+l-1)):0.0; }
            float as_denorm() const { if(is_double()) return as_double(); if(is_float()) return as_float(); return as_renorm(as_array_lbound(),as_array_ubound(),as_array_rest()); }

            double as_denorm_double() const { if(is_double()) return as_double(); if(is_float()) return as_float(); return as_denorm(); }
            float as_denorm_float() const { if(is_double()) return as_double(); if(is_float()) return as_float(); return as_denorm(); }

            double as_renorm_double(unsigned u, float min, float max, float rest) const { unsigned du=units(); if(du==BCTUNIT_GLOBAL || du==u) { if(is_double()) return as_double(); if(is_float()) return as_float(); } return denormalise(max,min,rest,as_norm()); }
            float as_renorm_float(unsigned u, float min, float max, float rest) const { unsigned du=units(); if(du==BCTUNIT_GLOBAL || du==u) { if(is_double()) return as_double(); if(is_float()) return as_float(); } return denormalise(max,min,rest,as_norm()); }
            float as_renorm(float min, float max, float rest) const { if(units()==BCTUNIT_GLOBAL) { if(is_float()) return as_float(); if(is_double()) return as_double(); } return denormalise(max,min,rest,as_norm()); }
            float as_renorm_float(float min, float max, float rest) const { return as_renorm(min,max,rest); }
            float as_renorm_double(float min, float max, float rest) const { if(units()==BCTUNIT_GLOBAL) { if(is_float()) return as_float(); if(is_double()) return as_double(); } return denormalise(max,min,rest,as_norm()); }

            long as_long() const { PIC_ASSERT(is_long()); return *(long *)host_data(); }
            const char *as_string() const { PIC_ASSERT(is_string()); return (const char *)host_data(); }
            unsigned as_stringlen() const { PIC_ASSERT(is_string()); return host_length(); }
            bool as_bool() const { PIC_ASSERT(is_bool()); return *(char *)host_data(); }
            const unsigned char *as_path() const { PIC_ASSERT(is_path()); return ((const unsigned char *)host_data())+1; }
            unsigned as_pathlen() const { PIC_ASSERT(is_path()); return host_length()-1; }
            unsigned as_pathchafflen() const { PIC_ASSERT(is_path()); return *(unsigned char *)(host_data()); }
            const unsigned char *as_pathgrist() const { PIC_ASSERT(is_path()); const unsigned char *p = (const unsigned char *)host_data(); unsigned c=*p; return &p[1+c]; }
            unsigned as_pathgristlen() const { PIC_ASSERT(is_path()); const unsigned char *p = (const unsigned char *)host_data(); unsigned c=*p; return host_length()-1-c; }
            const void *as_blob() const { PIC_ASSERT(is_blob()); return (const void *)host_data(); }
            unsigned as_bloblen() const { PIC_ASSERT(is_blob()); return host_length(); }
            std::string as_blob2() const { PIC_ASSERT(is_blob()); return std::string((const char *)host_data(),host_length()); }
            std::string as_stdstr() const { PIC_ASSERT(is_string()); return std::string(as_string(),as_stringlen()); }
            unsigned as_dict_nkeys() const;
            std::string as_dict_key(unsigned k) const;
            std::string as_pathstr() const { return std::string((const char *)as_path(),as_pathlen()); }

            std::string repr() const;
            long hash() const;

            bool issameas(const piw::data_base_t &other) const { return host_data()==other.host_data(); }

        protected:
            data_base_t(bct_data_t d): _rep(d) { }

            bct_data_t _rep;
    };

    class PIW_DECLSPEC_CLASS data_t : public data_base_t
    {
        public:
            data_t(): data_base_t(0) { }

            data_t(const data_t &d): data_base_t(d._rep)
            {
                piw_data_incref_atomic(_rep);
            }

            data_t &operator=(const data_t &d)
            {
                if(_rep!=d._rep)
                {
                    clear();
                    _rep=d._rep;
                    piw_data_incref_atomic(_rep);
                }
                
                return *this;
            }

            ~data_t() { clear(); }

            void clear() { piw_data_decref_atomic(_rep); _rep=0; }

            static data_t from_lent(bct_data_t r)
            {
#ifdef DEBUG_DATA_ATOMICITY
                if (r)
                {
                    if (!r->tid) r->tid = pic_current_threadid();
                    else if (r->nb_usage) data_atomicity_assertion(r);
                }
#endif
                piw_data_incref_atomic(r);
                
                return r;
            }

            static data_t from_given(bct_data_t r)
            {
#ifdef DEBUG_DATA_ATOMICITY
                if (r)
                {
                    if (!r->tid) r->tid = pic_current_threadid();
                    else if (r->nb_usage) data_atomicity_assertion(r);
                }
#endif
                return r;
            }

            bct_data_t lend() const { return _rep; }
            bct_data_t give() const { piw_data_incref_atomic(_rep); return _rep; }
            bct_data_t give_copy(unsigned nb) const { return data_base_t::give_copy(nb); }
            bct_data_t give_copy() const { return data_base_t::give_copy(PIC_ALLOC_NORMAL); }

            data_t copy() const { return data_t::from_given(give_copy()); }
            
            data_nb_t make_nb() const;
            data_t make_normal() const { return *this; }

            data_t as_dict_lookup(const std::string &key) const;
            data_t as_dict_value(unsigned k) const;

            data_t restamp(unsigned long long t) const;
            data_t realloc_real(unsigned nb) const;
            data_t realloc(unsigned nb) const { if(!_rep || !nb) return *this; if(bct_data_nb_mode(_rep)) return *this; return realloc_real(nb); }

        private:
            data_t(bct_data_t d): data_base_t(d) { }
    };

    class PIW_DECLSPEC_CLASS data_nb_t : public data_base_t
    {
        public:
            data_nb_t(): data_base_t(0) { }

            data_nb_t(const data_nb_t &d): data_base_t(d._rep)
            {
                piw_data_incref_fast(_rep);
            }

            data_nb_t &operator=(const data_nb_t &d)
            {
                if(_rep!=d._rep)
                {
                    clear();
                    _rep=d._rep;
                    piw_data_incref_fast(_rep);
                }
                return *this;
            }

            ~data_nb_t() { clear(); }

            void clear() { piw_data_decref_fast(_rep); _rep=0; }

            static data_nb_t from_lent(bct_data_t r)
            {
#ifdef DEBUG_DATA_ATOMICITY
                if (r)
                {
                    if (!r->tid) r->tid = pic_current_threadid();
                    else if (!r->nb_usage) data_atomicity_assertion(r);
                    r->nb_usage = true;
                }
#endif
                piw_data_incref_fast(r);
                
                return r;
            }

            static data_nb_t from_given(bct_data_t r)
            {
#ifdef DEBUG_DATA_ATOMICITY
                if (r)
                {
                    if (!r->tid) r->tid = pic_current_threadid();
                    else if (!r->nb_usage) data_atomicity_assertion(r);
                    r->nb_usage = true;
                }
                data_atomicity_assertion(r);
#endif

                return r;
            }

            bct_data_t lend() const { return _rep; }
            bct_data_t give() const { piw_data_incref_fast(_rep); return _rep; }
            bct_data_t give_copy(unsigned nb) const { return data_base_t::give_copy(nb); }
            bct_data_t give_copy() const { return data_base_t::give_copy(PIC_ALLOC_NB); }

            data_nb_t copy() const
            {
                return data_nb_t::from_given(give_copy());
            }
            
            data_nb_t make_nb() const { return *this; }
            data_t make_normal() const;

            data_nb_t as_dict_lookup(const std::string &key) const;
            data_nb_t as_dict_value(unsigned k) const;

            data_nb_t restamp(unsigned long long t) const;
            data_nb_t realloc_real(unsigned nb) const;
            data_nb_t realloc(unsigned nb) const { if(!_rep || !nb) return *this; if(bct_data_nb_mode(_rep)) return *this; return realloc_real(nb); }
        

        private:
            friend class dataholder_nb_t;

            data_nb_t(bct_data_t d): data_base_t(d) { }
    };

    typedef std::map<std::string,data_t> datamap_t;

    PIW_DECLSPEC_FUNC(bct_data_t) makecopy(unsigned nb, unsigned long long ts, bct_data_t &d);

    PIW_DECLSPEC_FUNC(data_t) makearray_ex(unsigned nb, unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs);
    PIW_DECLSPEC_FUNC(data_t) makenorm_ex(unsigned nb, unsigned long long ts, unsigned nfloats, float **pfloat,float **fs);
    PIW_DECLSPEC_FUNC(data_t) makefloat_bounded_ex(unsigned nb, float ubound, float lbound, float rest, float f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makedouble_bounded_ex(unsigned nb, float ubound, float lbound, float rest, double f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makelong_bounded_ex(unsigned nb, float ubound, float lbound, float rest, long f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makefloat_bounded_units_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, float f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makedouble_bounded_units_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, double f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makelong_bounded_units_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, long f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) makewire_ex(unsigned nb, unsigned dl, const unsigned char *dp);
    PIW_DECLSPEC_FUNC(data_t) pathnull_ex(unsigned nb, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) pathone_ex(unsigned nb, unsigned v, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) pathprepend_ex(unsigned nb, const data_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_t) pathtwo_ex(unsigned nb, unsigned v1, unsigned v2, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_t) pathprepend_grist_ex(unsigned nb, const data_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_t) pathappend_chaff_ex(unsigned nb, const data_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_t) pathtruncate_ex(unsigned nb, const data_t &d);
    PIW_DECLSPEC_FUNC(data_t) pathpretruncate_ex(unsigned nb, const data_t &d);
    PIW_DECLSPEC_FUNC(data_t) pathpretruncate_ex(unsigned nb, const data_t &d,unsigned l);
    PIW_DECLSPEC_FUNC(data_t) pathreplacegrist_ex(unsigned nb, const data_t &d, unsigned g);
    PIW_DECLSPEC_FUNC(data_t) pathgristpretruncate_ex(unsigned nb, const data_t &d);
    PIW_DECLSPEC_FUNC(data_t) makeblob_ex(unsigned nb, unsigned long long ts, unsigned size, unsigned char **pdata);
    PIW_DECLSPEC_FUNC(data_t) makedouble_ex(unsigned nb, double v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makefloat_ex(unsigned nb, float v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makelong_ex(unsigned nb, long v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makepath_ex(unsigned nb, const unsigned char *p, unsigned l, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) parsepath_ex(unsigned nb, const char *p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makestring_len_ex(unsigned nb, const char *p, unsigned len, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makestring_ex(unsigned nb, const char *p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makestring_ex(unsigned nb, const std::string &p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makebool_ex(unsigned nb, bool v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) makenull_ex(unsigned nb, unsigned long long ts=0);
    PIW_DECLSPEC_FUNC(data_t) dictnull_ex(unsigned nb, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_t) dictdel_ex(unsigned nb, const data_t &d, const std::string &k);
    PIW_DECLSPEC_FUNC(data_t) dictset_ex(unsigned nb, const data_t &d, const std::string &k,const data_t &v);
    PIW_DECLSPEC_FUNC(data_t) dictupdate_ex(unsigned nb, const data_t &d, const data_t &d2);

    inline data_t makewire(unsigned dl, const unsigned char *dp) { return makewire_ex(PIC_ALLOC_NORMAL,dl,dp); }
    inline data_t pathnull(unsigned long long t) { return pathnull_ex(PIC_ALLOC_NORMAL,t); }
    inline data_t pathone(unsigned v, unsigned long long t) { return pathone_ex(PIC_ALLOC_NORMAL,v,t); }
    inline data_t pathprepend(const data_t &d, unsigned p) { return pathprepend_ex(PIC_ALLOC_NORMAL,d,p); }
    inline data_t pathtwo(unsigned v1, unsigned v2, unsigned long long t) { return pathtwo_ex(PIC_ALLOC_NORMAL,v1,v2,t); }
    inline data_t pathprepend_grist(const data_t &d, unsigned p) { return pathprepend_grist_ex(PIC_ALLOC_NORMAL,d,p); }
    inline data_t pathappend_chaff(const data_t &d, unsigned p) { return pathappend_chaff_ex(PIC_ALLOC_NORMAL,d,p); }
    inline data_t pathtruncate(const data_t &d) { return pathtruncate_ex(PIC_ALLOC_NORMAL,d); }
    inline data_t pathpretruncate(const data_t &d) { return pathpretruncate_ex(PIC_ALLOC_NORMAL,d); }
    inline data_t pathpretruncate(const data_t &d,unsigned l) { return pathpretruncate_ex(PIC_ALLOC_NORMAL,d,l); }
    inline data_t pathgristpretruncate(const data_t &d) { return pathgristpretruncate_ex(PIC_ALLOC_NORMAL,d); }
    inline data_t pathreplacegrist(const data_t &d, unsigned g) { return pathreplacegrist_ex(PIC_ALLOC_NORMAL,d,g); }
    inline data_t dictnull(unsigned long long t) { return dictnull_ex(PIC_ALLOC_NORMAL,t); }
    inline data_t dictdel(const data_t &d, const std::string &k) { return dictdel_ex(PIC_ALLOC_NORMAL,d,k); }
    inline data_t dictset(const data_t &d, const std::string &k, const data_t &v) { return dictset_ex(PIC_ALLOC_NORMAL,d,k,v); }
    inline data_t dictupdate(const data_t &d, const data_t &d2) { return dictupdate_ex(PIC_ALLOC_NORMAL,d,d2); }
    inline data_t makeblob(unsigned long long ts, unsigned size, unsigned char **pdata) { return makeblob_ex(PIC_ALLOC_NORMAL,ts,size,pdata); }
    inline data_t makeblob2(const std::string &s, unsigned long long ts) { unsigned char *p; data_t d(makeblob_ex(PIC_ALLOC_NORMAL,ts,s.size(),&p)); memcpy(p,s.c_str(),s.size()); return d; }
    inline data_t makedouble(double v, unsigned long long t=0L) { return makedouble_ex(PIC_ALLOC_NORMAL,v,t); }
    inline data_t makefloat(float v, unsigned long long t=0L) { return makefloat_ex(PIC_ALLOC_NORMAL,v,t); }
    inline data_t makelong(long v, unsigned long long t=0L) { return makelong_ex(PIC_ALLOC_NORMAL,v,t); }
    inline data_t makepath(const unsigned char *p, unsigned l, unsigned long long t=0L) { return makepath_ex(PIC_ALLOC_NORMAL,p,l,t); }
    inline data_t parsepath(const char *p, unsigned long long t=0L) { return parsepath_ex(PIC_ALLOC_NORMAL,p,t); }
    inline data_t makestring_len(const char *p, unsigned len, unsigned long long t=0L) { return makestring_len_ex(PIC_ALLOC_NORMAL,p,len,t); }
    inline data_t makestring(const char *p, unsigned long long t=0L) { return makestring_ex(PIC_ALLOC_NORMAL,p,t); }
    inline data_t makestring(const std::string &p, unsigned long long t=0L) { return makestring_ex(PIC_ALLOC_NORMAL,p,t); }
    inline data_t makebool(bool v, unsigned long long t=0L) { return makebool_ex(PIC_ALLOC_NORMAL,v,t); }
    inline data_t makenull(unsigned long long ts=0) { return makenull_ex(PIC_ALLOC_NORMAL,ts); }
    inline data_t makearray(unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs) { return makearray_ex(PIC_ALLOC_NORMAL,ts,ubound,lbound,rest,nfloats,pfloat,fs); }
    inline data_t makenorm(unsigned long long ts, unsigned nfloats, float **pfloat,float **fs) { return makearray_ex(PIC_ALLOC_NORMAL,ts,1,-1,0,nfloats,pfloat,fs); }
    inline data_t makefloat_bounded(float ubound, float lbound, float rest, float f, unsigned long long t) { return makefloat_bounded_ex(PIC_ALLOC_NORMAL,ubound,lbound,rest,f,t); }
    inline data_t makedouble_bounded(float ubound, float lbound, float rest, double f, unsigned long long t) { return makedouble_bounded_ex(PIC_ALLOC_NORMAL,ubound,lbound,rest,f,t); }
    inline data_t makelong_bounded(float ubound, float lbound, float rest, long f, unsigned long long t) { return makelong_bounded_ex(PIC_ALLOC_NORMAL,ubound,lbound,rest,f,t); }
    inline data_t makefloat_bounded_units(unsigned u, float ubound, float lbound, float rest, float f, unsigned long long t) { return makefloat_bounded_units_ex(PIC_ALLOC_NORMAL,u,ubound,lbound,rest,f,t); }
    inline data_t makedouble_bounded_units(unsigned u, float ubound, float lbound, float rest, double f, unsigned long long t) { return makedouble_bounded_units_ex(PIC_ALLOC_NORMAL,u,ubound,lbound,rest,f,t); }
    inline data_t makelong_bounded_units(unsigned u, float ubound, float lbound, float rest, long f, unsigned long long t) { return makelong_bounded_units_ex(PIC_ALLOC_NORMAL,u,ubound,lbound,rest,f,t); }

    PIW_DECLSPEC_FUNC(data_nb_t) makearray_nb_ex(unsigned nb, unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs);
    PIW_DECLSPEC_FUNC(data_nb_t) makenorm_nb_ex(unsigned nb, unsigned long long ts, unsigned nfloats, float **pfloat,float **fs);
    PIW_DECLSPEC_FUNC(data_nb_t) makefloat_bounded_nb_ex(unsigned nb, float ubound, float lbound, float rest, float f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makedouble_bounded_nb_ex(unsigned nb, float ubound, float lbound, float rest, double f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makelong_bounded_nb_ex(unsigned nb, float ubound, float lbound, float rest, long f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makefloat_bounded_units_nb_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, float f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makedouble_bounded_units_nb_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, double f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makelong_bounded_units_nb_ex(unsigned nb, unsigned u, float ubound, float lbound, float rest, long f, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) makewire_nb_ex(unsigned nb, unsigned dl, const unsigned char *dp);
    PIW_DECLSPEC_FUNC(data_nb_t) pathnull_nb_ex(unsigned nb, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) pathone_nb_ex(unsigned nb, unsigned v, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) pathprepend_nb_ex(unsigned nb, const data_nb_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_nb_t) pathtwo_nb_ex(unsigned nb, unsigned v1, unsigned v2, unsigned long long t);
    PIW_DECLSPEC_FUNC(data_nb_t) pathprepend_grist_nb_ex(unsigned nb, const data_nb_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_nb_t) pathappend_chaff_nb_ex(unsigned nb, const data_nb_t &d, unsigned p);
    PIW_DECLSPEC_FUNC(data_nb_t) pathtruncate_nb_ex(unsigned nb, const data_nb_t &d);
    PIW_DECLSPEC_FUNC(data_nb_t) pathpretruncate_nb_ex(unsigned nb, const data_nb_t &d);
    PIW_DECLSPEC_FUNC(data_nb_t) pathpretruncate_nb_ex(unsigned nb, const data_nb_t &d,unsigned l);
    PIW_DECLSPEC_FUNC(data_nb_t) pathreplacegrist_nb_ex(unsigned nb, const data_nb_t &d, unsigned g);
    PIW_DECLSPEC_FUNC(data_nb_t) pathgristpretruncate_nb_ex(unsigned nb, const data_nb_t &d);
    PIW_DECLSPEC_FUNC(data_nb_t) makeblob_nb_ex(unsigned nb, unsigned long long ts, unsigned size, unsigned char **pdata);
    PIW_DECLSPEC_FUNC(data_nb_t) makedouble_nb_ex(unsigned nb ,double v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makefloat_nb_ex(unsigned nb, float v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makelong_nb_ex(unsigned nb, long v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makepath_nb_ex(unsigned nb, const unsigned char *p, unsigned l, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) parsepath_nb_ex(unsigned nb,const char *p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makestring_len_nb_ex(unsigned nb, const char *p, unsigned len, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makestring_nb_ex(unsigned nb, const char *p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makestring_nb_ex(unsigned nb, const std::string &p, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makebool_nb_ex(unsigned nb, bool v, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) makenull_nb_ex(unsigned nb, unsigned long long ts=0);
    PIW_DECLSPEC_FUNC(data_nb_t) dictnull_nb_ex(unsigned nb, unsigned long long t=0L);
    PIW_DECLSPEC_FUNC(data_nb_t) dictdel_nb_ex(unsigned nb, const data_nb_t &d, const std::string &k);
    PIW_DECLSPEC_FUNC(data_nb_t) dictset_nb_ex(unsigned nb, const data_nb_t &d, const std::string &k, const data_nb_t &v);
    PIW_DECLSPEC_FUNC(data_nb_t) dictupdate_nb_ex(unsigned nb, const data_nb_t &d, const data_nb_t &d2);

    inline data_nb_t makewire_nb(unsigned dl, const unsigned char *dp) { return makewire_nb_ex(PIC_ALLOC_NB,dl,dp); }
    inline data_nb_t pathnull_nb(unsigned long long t) { return pathnull_nb_ex(PIC_ALLOC_NB,t); }
    inline data_nb_t pathone_nb(unsigned v, unsigned long long t) { return pathone_nb_ex(PIC_ALLOC_NB,v,t); }
    inline data_nb_t pathprepend_nb(const data_nb_t &d, unsigned p) { return pathprepend_nb_ex(PIC_ALLOC_NB,d,p); }
    inline data_nb_t pathtwo_nb(unsigned v1, unsigned v2, unsigned long long t) { return pathtwo_nb_ex(PIC_ALLOC_NB,v1,v2,t); }
    inline data_nb_t pathprepend_grist_nb(const data_nb_t &d, unsigned p) { return pathprepend_grist_nb_ex(PIC_ALLOC_NB,d,p); }
    inline data_nb_t pathappend_chaff_nb(const data_nb_t &d, unsigned p) { return pathappend_chaff_nb_ex(PIC_ALLOC_NB,d,p); }
    inline data_nb_t pathtruncate_nb(const data_nb_t &d) { return pathtruncate_nb_ex(PIC_ALLOC_NB,d); }
    inline data_nb_t pathpretruncate_nb(const data_nb_t &d) { return pathpretruncate_nb_ex(PIC_ALLOC_NB,d); }
    inline data_nb_t pathpretruncate_nb(const data_nb_t &d,unsigned l) { return pathpretruncate_nb_ex(PIC_ALLOC_NB,d,l); }
    inline data_nb_t pathgristpretruncate_nb(const data_nb_t &d) { return pathgristpretruncate_nb_ex(PIC_ALLOC_NB,d); }
    inline data_nb_t pathreplacegrist_nb(const data_nb_t &d, unsigned g) { return pathreplacegrist_nb_ex(PIC_ALLOC_NB,d,g); }
    inline data_nb_t dictnull_nb(unsigned long long t) { return dictnull_nb_ex(PIC_ALLOC_NB,t); }
    inline data_nb_t dictdel_nb(const data_nb_t &d, const std::string &k) { return dictdel_nb_ex(PIC_ALLOC_NB,d,k); }
    inline data_nb_t dictset_nb(const data_nb_t &d, const std::string &k, const data_nb_t &v) { return dictset_nb_ex(PIC_ALLOC_NB,d,k,v); }
    inline data_nb_t dictupdate_nb(const data_nb_t &d, const data_nb_t &d2) { return dictupdate_nb_ex(PIC_ALLOC_NB,d,d2); }
    inline data_nb_t makeblob_nb(unsigned long long ts, unsigned size, unsigned char **pdata) { return makeblob_nb_ex(PIC_ALLOC_NB,ts,size,pdata); }
    inline data_nb_t makedouble_nb(double v, unsigned long long t=0L) { return makedouble_nb_ex(PIC_ALLOC_NB,v,t); }
    inline data_nb_t makefloat_nb(float v, unsigned long long t=0L) { return makefloat_nb_ex(PIC_ALLOC_NB,v,t); }
    inline data_nb_t makelong_nb(long v, unsigned long long t=0L) { return makelong_nb_ex(PIC_ALLOC_NB,v,t); }
    inline data_nb_t makepath_nb(const unsigned char *p, unsigned l, unsigned long long t=0L) { return makepath_nb_ex(PIC_ALLOC_NB,p,l,t); }
    inline data_nb_t parsepath_nb(const char *p, unsigned long long t=0L) { return parsepath_nb_ex(PIC_ALLOC_NB,p,t); }
    inline data_nb_t makestring_len_nb(const char *p, unsigned len, unsigned long long t=0L) { return makestring_len_nb_ex(PIC_ALLOC_NB,p,len,t); }
    inline data_nb_t makestring_nb(const char *p, unsigned long long t=0L) { return makestring_nb_ex(PIC_ALLOC_NB,p,t); }
    inline data_nb_t makestring_nb(const std::string &p, unsigned long long t=0L) { return makestring_nb_ex(PIC_ALLOC_NB,p,t); }
    inline data_nb_t makebool_nb(bool v, unsigned long long t=0L) { return makebool_nb_ex(PIC_ALLOC_NB,v,t); }
    inline data_nb_t makenull_nb(unsigned long long ts=0) { return makenull_nb_ex(PIC_ALLOC_NB,ts); }
    inline data_nb_t makearray_nb(unsigned long long ts, float ubound, float lbound, float rest, unsigned nfloats, float **pfloat, float **fs) { return makearray_nb_ex(PIC_ALLOC_NB,ts,ubound,lbound,rest,nfloats,pfloat,fs); }
    inline data_nb_t makenorm_nb(unsigned long long ts, unsigned nfloats, float **pfloat,float **fs) { return makenorm_nb_ex(PIC_ALLOC_NB,ts,nfloats,pfloat,fs); }
    inline data_nb_t makefloat_bounded_nb(float ubound, float lbound, float rest, float f, unsigned long long t) { return makefloat_bounded_nb_ex(PIC_ALLOC_NB,ubound,lbound,rest,f,t); }
    inline data_nb_t makefloat_bounded_units_nb(unsigned u, float ubound, float lbound, float rest, float f, unsigned long long t) { return makefloat_bounded_units_nb_ex(PIC_ALLOC_NB,u,ubound,lbound,rest,f,t); }
    inline data_nb_t makedouble_bounded_nb(float ubound, float lbound, float rest, double f, unsigned long long t) { return makedouble_bounded_nb_ex(PIC_ALLOC_NB,ubound,lbound,rest,f,t); }
    inline data_nb_t makedouble_bounded_units_nb(unsigned u, float ubound, float lbound, float rest, double f, unsigned long long t) { return makedouble_bounded_units_nb_ex(PIC_ALLOC_NB,u,ubound,lbound,rest,f,t); }
    inline data_nb_t makelong_bounded_nb(float ubound, float lbound, float rest, long f, unsigned long long t) { return makelong_bounded_nb_ex(PIC_ALLOC_NB,ubound,lbound,rest,f,t); }
    inline data_nb_t makelong_bounded_units_nb(unsigned u, float ubound, float lbound, float rest, long f, unsigned long long t) { return makelong_bounded_units_nb_ex(PIC_ALLOC_NB,u,ubound,lbound,rest,f,t); }

    typedef pic::functor_t<data_t(const data_t &)> d2d_t;
    typedef pic::functor_t<void(const data_t &)> change_t;
    typedef pic::functor_t<data_t(const data_t &,const data_t &)> dd2d_t;
    typedef pic::functor_t<bool(const data_t &)> d2b_t;
    
    typedef pic::functor_t<void(const data_nb_t &)> change_nb_t;
    typedef pic::functor_t<data_nb_t(const data_nb_t &)> d2d_nb_t;
    typedef pic::functor_t<data_nb_t(const data_nb_t &,const data_nb_t &)> dd2d_nb_t;
    typedef pic::functor_t<bool(const data_nb_t &)> d2b_nb_t;

    PIW_DECLSPEC_FUNC(piw::change_t) make_change_normal(const piw::change_nb_t &);
    PIW_DECLSPEC_FUNC(piw::change_nb_t) make_change_nb(const piw::change_t &);

    PIW_DECLSPEC_FUNC(change_t) changelist();
    PIW_DECLSPEC_FUNC(void) changelist_connect(change_t &, const change_t &);
    PIW_DECLSPEC_FUNC(void) changelist_disconnect(change_t &, const change_t &);

    PIW_DECLSPEC_FUNC(change_nb_t) changelist_nb();
    PIW_DECLSPEC_FUNC(void) changelist_connect_nb(change_nb_t &, const change_nb_t &);
    PIW_DECLSPEC_FUNC(void) changelist_disconnect_nb(change_nb_t &, const change_nb_t &);

    PIW_DECLSPEC_FUNC(change_t) indirectchange();
    PIW_DECLSPEC_FUNC(void) indirectchange_set(change_t &, const change_t &);
    PIW_DECLSPEC_FUNC(void) indirectchange_clear(change_t &);

    PIW_DECLSPEC_FUNC(change_nb_t) indirectchange_nb();
    PIW_DECLSPEC_FUNC(void) indirectchange_set_nb(change_nb_t &, const change_nb_t &);
    PIW_DECLSPEC_FUNC(void) indirectchange_clear_nb(change_nb_t &);

    PIW_DECLSPEC_FUNC(change_t) trigger(const change_nb_t &c, const data_nb_t &v);

    PIW_DECLSPEC_FUNC(change_t) change2(const change_t &c1, const change_t &c2);
    PIW_DECLSPEC_FUNC(change_nb_t) change2_nb(const change_t &c1, const change_t &c2);
    PIW_DECLSPEC_FUNC(change_nb_t) change_nb2(const change_nb_t &c1, const change_nb_t &c2);

    class PIW_DECLSPEC_CLASS dataholder_nb_t
    {
        public:
            dataholder_nb_t();
            explicit dataholder_nb_t(const data_nb_t &d);
            explicit dataholder_nb_t(const data_t &d);
            ~dataholder_nb_t();
            
            dataholder_nb_t(const dataholder_nb_t &h);
            dataholder_nb_t &operator=(const dataholder_nb_t &h);

            operator data_nb_t() const;
            bool operator==(const dataholder_nb_t &d) const { return d.get()==get(); }

            void set_normal(const data_t &d);
            void set_nb(const data_nb_t &d);
            void clear();
            const data_nb_t get() const;

        private:
            static int copy_constructor__(void *self_, void *arg_);
            static int assignment__(void *self_, void *arg_);
            static int clear__(void *self_, void *arg_);
            static int set_normal__(void *self_, void *arg_);

            bct_data_t data_;
    };

    class PIW_DECLSPEC_CLASS datadrop_t
    {
        public:
            datadrop_t() { set(data_nb_t()); }
            datadrop_t(const data_nb_t &d) { set(d); }
            ~datadrop_t() { set(data_nb_t()); }
            void set(const data_nb_t &d)
            {
                drop_.set(d.give_copy());
            }
            data_t get()
            {
                return data_t::from_given(drop_.get());
            }
        private:
            static void data_incref(bct_data_t d) { piw_data_incref_atomic(d); }
            static void data_decref(bct_data_t d) { piw_data_decref_atomic(d); }
            pic::datadrop_t<bct_data_t,data_incref,data_decref> drop_;
    };


    template <class T> class fullprinter_t
    {
        public:
            fullprinter_t(const T &d): data_(d) {}
            T data_;
    };

    struct path_less
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.compare_path(b)<0; }
    };

    struct path_less_lexicographic
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.compare_path_lexicographic(b)<0; }
    };

    struct path_greater_lexicographic
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.compare_path_lexicographic(b)>0; }
    };

    struct grist_less
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.compare_grist(b)<0; }
    };

    struct notime_less
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.compare(b,false)<0; }
    };

    struct identity_less
    {
        bool operator()(const data_base_t &a, const data_base_t &b) const { return a.lend()<b.lend(); }
    };

    PIW_DECLSPEC_FUNC(d2d_nb_t) dict_merger(const data_t &override);
}

PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &o, const piw::dataholder_nb_t &d);
PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &o, const piw::data_base_t &p);
PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &o, const piw::fullprinter_t<piw::data_t> &p);
PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &o, const piw::fullprinter_t<piw::data_nb_t> &p);

#endif
