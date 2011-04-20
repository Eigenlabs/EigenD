
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

#ifndef __PIW_SAMPLE__
#define __PIW_SAMPLE__
#include "piw_exports.h"
#include <picross/pic_ref.h>
#include <piw/piw_data.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS samplearray_t: public pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        public:
            class impl_t;
        public:
            samplearray_t(const char *file, unsigned p, unsigned s);
            ~samplearray_t();

            impl_t *impl() { return impl_; }
        private:
            impl_t *impl_;
    };

    typedef pic::ref_t<samplearray_t> samplearrayref_t;

    inline samplearrayref_t create_samplearray(const char *f,unsigned p,unsigned l) { return pic::ref(new samplearray_t(f,p,l)); }

    struct PIW_DECLSPEC_CLASS sample_t: pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        public:
            struct impl_t;

        public:
            sample_t(const samplearrayref_t &d,unsigned s, unsigned e, unsigned ls, unsigned le, float sr, float rf, float att);
            ~sample_t();

            samplearrayref_t data();
            unsigned start() const;
            unsigned end() const;
            unsigned loopstart() const;
            unsigned loopend() const;
            float samplerate() const;
            float rootfreq() const;
            float attenuation() const;
            impl_t *impl() { return impl_; }

        private:
            impl_t *impl_;

    };

    typedef pic::ref_t<sample_t> sampleref_t;

    struct PIW_DECLSPEC_CLASS samplereader_t: pic::nocopy_t, virtual public pic::lckobject_t
    {
        public:
            class rimpl_t;

        public:
            samplereader_t(const sampleref_t &sample);
            ~samplereader_t();
            const short *bufptr(unsigned offset);

        private:
            rimpl_t *impl_;
    };

    inline sampleref_t create_sample(const samplearrayref_t &a,unsigned s, unsigned e, unsigned ls, unsigned le, float sr, float rf, float att) { return pic::ref(new sample_t(a,s,e,ls,le,sr,rf,att)); }

    struct PIW_DECLSPEC_CLASS xzone_t: pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        xzone_t(float fn,float fx,float vn,float vx,float de_,float a_,float h_,float dc_,float s_,float r_,float p_,const sampleref_t &c_): fmin(fn),fmax(fx),vmin(vn),vmax(vx),de(de_),a(a_),h(h_),dc(dc_),s(s_),r(r_),p(p_),c(c_) {}
        float fmin,fmax;
        float vmin,vmax;
        float de,a,h,dc,s,r;
        float p;
        sampleref_t c;
    };

    typedef pic::ref_t<xzone_t> zoneref_t;

    inline zoneref_t create_zone(float fn,float fx,float vn,float vx,float de_,float a_,float h_,float dc_,float s_,float r_,float p_,const sampleref_t &c_)
    {
        return pic::ref(new xzone_t(fn,fx,vn,vx,de_,a_,h_,dc_,s_,r_,p_,c_));
    }

    struct PIW_DECLSPEC_CLASS xvoice_t: pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        void add_zone(const zoneref_t &z) { zones.push_back(z); }
        void envelope(float v,float f,float *dep,float *ap,float *hp,float *dcp,float *sp,float *rp);
        pic::lcklist_t<zoneref_t>::nbtype zones;
    };

    typedef pic::ref_t<xvoice_t> voiceref_t;
    inline voiceref_t create_voice() { return pic::ref(new xvoice_t); }

    typedef std::pair<float,float> zonekey_t;
    typedef pic::lckmultimap_t<zonekey_t,zoneref_t>::nbtype zonemap_t;

    struct PIW_DECLSPEC_CLASS xpreset_t: pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        xpreset_t();
        void add_zone(const zoneref_t &z);
        voiceref_t find_zone(float v, float f) const;

        zonemap_t zones;
        float minv,minf;
        float maxv,maxf;
    };

    typedef pic::ref_t<xpreset_t> presetref_t;
    
    inline presetref_t create_preset() { return pic::ref(new xpreset_t); }
}

#endif
