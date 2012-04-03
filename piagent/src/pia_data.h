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

#ifndef __PIA_SRC_DATA__
#define __PIA_SRC_DATA__

#include <string>
#include <sstream>

#include <picross/pic_fastalloc.h>
#include <pibelcanto/plugin.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_error.h>
#include <picross/pic_flipflop.h>
#include <piembedded/pie_iostream.h>
#include <piembedded/pie_print.h>


inline void data_atomicity_assertion(bct_data_t d)
{
#ifdef DEBUG_DATA_ATOMICITY
    if(d && d->tid && !pic_threadid_equal(d->tid, pic_current_threadid()))
    {
        std::stringstream oss;
        oss << "pia_data thread mismatch ";
        oss << d->tid;
        oss << "(data thread) != ";
        oss << pic_current_threadid();
        oss << "(current thread)";
        PIC_THROW(oss.str().c_str());
    }
#endif
}

inline void pia_data_incref_fast(bct_data_t d)
{
    if(d)
    {
        data_atomicity_assertion(d);

        ++d->count;
    }
}

inline void pia_data_decref_fast(bct_data_t d)
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

inline void pia_data_incref_atomic(bct_data_t d) { if(d) pic_atomicinc(&d->count); }
inline void pia_data_decref_atomic(bct_data_t d) { if(d) if(pic_atomicdec(&d->count)==0) bct_data_free(d); }

bct_data_t allocate_host_raw(pic::nballocator_t *a, unsigned nb, unsigned long long ts, float u, float l, float rst,unsigned t, unsigned sl, unsigned char **hp, unsigned vl, float **vp);
bct_data_t allocate_wire_raw(pic::nballocator_t *a, unsigned nb, unsigned wl, const unsigned char *wp);

class pia_data_base_t
{
	public:
		virtual ~pia_data_base_t() { }

        bool operator==(const pia_data_base_t &other) const { return compare(other)==0; }
        bool operator!=(const pia_data_base_t &other) const { return compare(other)!=0; }
        bool operator<(const pia_data_base_t &other) const { return compare(other)<0; }

        int compare_path(const pia_data_base_t &other) const;
        int compare(const pia_data_base_t &other,bool ts=true) const;

        operator bool() const { return data_ && bct_data_type(data_)!=BCTVTYPE_NULL; }

        long hash() const;
#ifdef DEBUG_DATA_ATOMICITY
        bool nb_usage() const { if(!data_) return false; return data_->nb_usage; } 
#endif

        virtual bct_data_t lend() const = 0;
        virtual bct_data_t give() const = 0;
        virtual bct_data_t give_copy(pic::nballocator_t *a, unsigned nb) const;

        const char *asstring() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_STRING); return (const char *)hostdata(); }
        unsigned asstringlen() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_STRING); return hostlen(); }
        const unsigned char *aspath() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_PATH); return (const unsigned char *)hostdata()+1; }
        const void *asblob() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_BLOB); return (const void *)hostdata(); }
        bool asbool() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_BOOL); return (*hostdata())?true:false; }
        unsigned aspathlen() const { PIC_ASSERT(bct_data_type(data_)==BCTVTYPE_PATH); return hostlen()-1; }

        unsigned type() const { return bct_data_type(data_); }
        unsigned units() const { return bct_data_units(data_); }
        unsigned hostlen() const { return bct_data_len(data_); }
        unsigned wirelen() const { return bct_data_wirelen(data_); }
        const unsigned char *hostdata() const { return bct_data_data(data_); }
        const unsigned char *wiredata() const { return bct_data_wiredata(data_); }
        unsigned long long time() const { return bct_data_time(data_); }
        float ubound() const { return bct_data_ubound(data_); }
        float lbound() const { return bct_data_lbound(data_); }
        float rest() const { return bct_data_rest(data_); }
        unsigned veclen() const { return bct_data_veclen(data_); }
        const float *vec() const { return bct_data_vector(data_); }

        bool is_null() const { return type()==BCTVTYPE_NULL; }

    protected:
		pia_data_base_t(bct_data_t d): data_(d) { }
        bct_data_t data_;
};

class pia_data_t: public pia_data_base_t
{
    public:
		pia_data_t(): pia_data_base_t(0) {}
		pia_data_t(const pia_data_t &d): pia_data_base_t(d.data_)
        {
            pia_data_incref_atomic(data_);
        }
		~pia_data_t()
        {
            pia_data_decref_atomic(data_);
            data_ = 0;
        }

        pia_data_t &operator=(const pia_data_t &d)
        {
            if(data_!=d.data_)
            {
                pia_data_decref_atomic(data_);
                data_=d.data_;
                pia_data_incref_atomic(data_);
            }
            return *this;
        }

        static pia_data_t from_lent(bct_data_t d)
        {
#ifdef DEBUG_DATA_ATOMICITY
            if (d)
            {
                if (!d->tid) d->tid = pic_current_threadid();
                else if (d->nb_usage) data_atomicity_assertion(d);
            }
#endif
            pia_data_incref_atomic(d);
            return pia_data_t(d);
        }

        static pia_data_t from_given(bct_data_t d)
        {
#ifdef DEBUG_DATA_ATOMICITY
            if (d)
            {
                if (!d->tid) d->tid = pic_current_threadid();
                else if (d->nb_usage) data_atomicity_assertion(d);
            }
#endif
            return pia_data_t(d);
        }

        pia_data_t copy(pic::nballocator_t *a, unsigned nb) const { return pia_data_t::from_given(give_copy(a, nb)); }

        bct_data_t lend() const { return data_; }
        bct_data_t give() const { pia_data_incref_atomic(data_); return data_; }
        bct_data_t give_copy(pic::nballocator_t *a) const { return pia_data_base_t::give_copy(a, PIC_ALLOC_NORMAL); }
        bct_data_t give_copy(pic::nballocator_t *a, unsigned nb) const { return pia_data_base_t::give_copy(a, nb); }

        static pia_data_t allocate_host(pic::nballocator_t *, unsigned nb, unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp);
        static pia_data_t allocate_wire(pic::nballocator_t *, unsigned nb, unsigned dl, const unsigned char *dp);
        static pia_data_t allocate_path(pic::nballocator_t *, unsigned pl, const unsigned char *dp, unsigned long long ts=0ULL);
        static pia_data_t allocate_string(pic::nballocator_t *, const char *s, unsigned sl,unsigned long long ts = 0ULL);
        static pia_data_t allocate_cstring(pic::nballocator_t *, const char *s, unsigned sl,unsigned long long ts = 0ULL);
        static pia_data_t allocate_cstring_nb(pic::nballocator_t *, const char *s, unsigned sl,unsigned long long ts = 0ULL);
        static pia_data_t allocate_bool(pic::nballocator_t *, bool v, unsigned long long ts = 0ULL);
        static pia_data_t allocate_buffer(pic::nballocator_t *, unsigned dl, unsigned char **dp);
        static pia_data_t path_append(pic::nballocator_t *, const pia_data_t &p, unsigned e);

    private:
		pia_data_t(bct_data_t d): pia_data_base_t(d) { }
};

class pia_data_nb_t: public pia_data_base_t
{
    public:
		pia_data_nb_t(): pia_data_base_t(0)
        {
        }

		pia_data_nb_t(const pia_data_nb_t &d): pia_data_base_t(d.data_)
        {
            pia_data_incref_fast(data_);
        }

		~pia_data_nb_t()
        {
            pia_data_decref_fast(data_);
            data_ = 0;
        }

        pia_data_nb_t &operator=(const pia_data_nb_t &d)
        {
            if(data_!=d.data_)
            {
                pia_data_decref_fast(data_);
                data_=d.data_;
                pia_data_incref_fast(data_);
            }
            return *this;
        }

        static pia_data_nb_t from_lent(bct_data_t d)
        {
#ifdef DEBUG_DATA_ATOMICITY
            if (d)
            {
                if (!d->tid) d->tid = pic_current_threadid();
                else if (!d->nb_usage) data_atomicity_assertion(d);
                d->nb_usage = true;
            }
#endif
            pia_data_incref_fast(d);
            return pia_data_nb_t(d);
        }

        static pia_data_nb_t from_given(bct_data_t d)
        {
#ifdef DEBUG_DATA_ATOMICITY
            if (d)
            {
                if (!d->tid) d->tid = pic_current_threadid();
                else if (!d->nb_usage) data_atomicity_assertion(d);
                d->nb_usage = true; 
            }
            data_atomicity_assertion(d);
#endif
            return pia_data_nb_t(d);
        }

        pia_data_t make_normal(pic::nballocator_t *a, unsigned nb) const { return pia_data_t::from_given(give_copy(a, nb)); }

        bct_data_t lend() const { return data_; }
        bct_data_t give() const { pia_data_incref_fast(data_); return data_; }
        bct_data_t give_copy(pic::nballocator_t *a) const { return pia_data_base_t::give_copy(a, PIC_ALLOC_NB); }
        bct_data_t give_copy(pic::nballocator_t *a, unsigned nb) const { return pia_data_base_t::give_copy(a, nb); }

        static pia_data_nb_t allocate_host(pic::nballocator_t *, unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp);
        static pia_data_nb_t allocate_wire(pic::nballocator_t *, unsigned dl, const unsigned char *dp);

    private:
		pia_data_nb_t(bct_data_t d): pia_data_base_t(d) { }
};

struct pia_notime_less
{
    bool operator()(const pia_data_t &a, const pia_data_t &b) const { return a.compare(b,false)<0; }
};

struct pia_path_less
{
    bool operator()(const pia_data_t &a, const pia_data_t &b) const { return a.compare_path(b)<0; }
};

class pia_dataholder_nb_t
{
    public:
        pia_dataholder_nb_t();
        ~pia_dataholder_nb_t();
            
        pia_dataholder_nb_t(const pia_dataholder_nb_t &h);
        pia_dataholder_nb_t &operator=(const pia_dataholder_nb_t &h);

        operator pia_data_nb_t() const;
        bool operator==(const pia_dataholder_nb_t &d) const { return d.get_nb()==get_nb(); }

        void set(const pia_data_nb_t &d);
        void clear();
        const pia_data_nb_t get_nb() const;
        const pia_data_t get_normal(pic::nballocator_t *a) const;

    private:

        bct_data_t data_;
};

class pia_datadrop_t: public pic::nocopy_t
{
    public:
        pia_datadrop_t(pic::nballocator_t *a) : allocator_(a) { set(pia_data_t()); }
        pia_datadrop_t(pic::nballocator_t *a, const pia_data_t &d) : allocator_(a) { set(d); }
        ~pia_datadrop_t() { set(pia_data_t()); }
        void set(const pia_data_t &d)
        {
            drop_.set(d.give_copy(allocator_, PIC_ALLOC_NB));
        }
        pia_data_nb_t get()
        {
            return pia_data_nb_t::from_given(drop_.get());
        }
    private:
        static void data_incref(bct_data_t d) { pia_data_incref_atomic(d); }
        static void data_decref(bct_data_t d) { pia_data_decref_atomic(d); }
        pic::datadrop_t<bct_data_t,data_incref,data_decref> drop_;
        pic::nballocator_t * const allocator_;
};

inline std::ostream &operator<<(std::ostream &o, const pia_data_t &d)
{
    pie_print(d.wirelen(),d.wiredata(), pie::ostreamwriter, &o);
    return o;
}

#endif
