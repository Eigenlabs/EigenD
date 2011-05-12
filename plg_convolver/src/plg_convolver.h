
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


#ifndef __PIW_CONVOLVER__
#define __PIW_CONVOLVER__

#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <picross/pic_log.h>
#include <convolver_exports.h>

namespace plg_convolver
{
    struct CONVOLVER_DECLSPEC_CLASS samplearray2_t : pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        samplearray2_t(unsigned s) : size(s)
        {
            pic::logmsg() << "allocating " << sizeof(float)*size << " for sample";
            data = (float *)malloc(sizeof(float)*size);
            PIC_ASSERT(data);
        }

        ~samplearray2_t()
        {
            pic::logmsg() << "freeing " << sizeof(float)*size << " for sample";
            free(data);
        }

        unsigned long size;
        float *data;
        float rate;
        unsigned chans;
    };

    typedef pic::ref_t<samplearray2_t> samplearray2ref_t;

    CONVOLVER_DECLSPEC_FUNC(samplearray2ref_t) canonicalise_samples(const std::string &data,float rate,unsigned chans,unsigned bitsPerSamp);

    class CONVOLVER_DECLSPEC_CLASS convolver_t : public pic::nocopy_t
    {
        public:
            convolver_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~convolver_t();

            piw::cookie_t cookie();
            void set_impulse_response(samplearray2ref_t &imp_resp);
            void set_mono_processing(bool mono);
            void set_enable(bool enable);
            void set_enable_time(float time);

            class impl_t;
        private:
            impl_t *impl_;

    };

}

#endif
