
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


#ifndef __PIW_PHASE__
#define __PIW_PHASE__
#include "piw_exports.h"
#include <picross/pic_config.h>
#include <picross/pic_float.h>
#include <piw/piw_sample.h>
#include <picross/pic_stdint.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS phase_t
    {
        public:

            phase_t(): cbuff1_a(0), cbuff2_a(0) { b64=0; }
            phase_t(int32_t b, uint32_t f): cbuff1_a(0), cbuff2_a(0) { b32.index=b; b32.fract=0; }

            phase_t(double b): cbuff1_a(0), cbuff2_a(0)
            {
              b32.index = (int32_t) (b);
              b32.fract = (uint32_t) (((double)(b) - (double)(b32.index)) * ((double)4294967296.0));
            }

            phase_t(const phase_t &p): cbuff1_a(0), cbuff2_a(0) { b64=p.b64; }
            phase_t &operator=(const phase_t &p) { b64=p.b64; return *this; }

            uint32_t index() const { return b32.index; }
            uint32_t fract() const { return b32.fract; }
            double   asdouble() const { return ((double)(b32.index) + ((double)(b32.fract) / ((double)4294967296.0))); }

            void advance(const phase_t &p) { b64+=p.b64; }
            phase_t &operator-=(uint32_t b) { b32.index-=b; return *this; }
            phase_t &operator+=(uint32_t b) { b32.index+=b; return *this; }

            int32_t iincr() { return b32.index++; }

            int steps(double index, double incr) { return (int)(((double)(index) - asdouble()) / (double)incr); }

            void interpolate_calc(const phase_t &dsp_phase_incr,unsigned dspl,piw::samplereader_t *reader,float atten)
            {
                if (!cbuff1_a || cbuff1.as_arraylen() < dspl)
                {
                    float *bs;
                    cbuff1 = piw::makearray_nb(0,1,0,0,dspl,&cbuff1_a,&bs);
                    cbuff2 = piw::makearray_nb(0,1,0,0,dspl,&cbuff2_a,&bs);
                }

                /* Check for a special case: The current phase falls directly on an
                 * original sample.  Also, the stepsize per output sample is exactly
                 * one sample, no fractional part.  In other words: The sample is
                 * played back at normal phase and root pitch.  => No interpolation
                 * needed.
                 */

                uint32_t origin = index();
                const short *buf = reader->bufptr(origin);

                if ((fract() == 0) && (dsp_phase_incr.fract() == 0) && (dsp_phase_incr.index() == 1))
                {
                    for (uint32_t bi = 0; bi < dspl; bi++)
                    {
                        cbuff1_a[bi] = (float)(buf[iincr()-origin]);
                    }
                    float d = 32678.0f;
                    pic::vector::vectdiv(&d,0,&cbuff1_a[0],1,&cbuff1_a[0],1,dspl);
                }
                else
                {
                    for (uint32_t bi = 0; bi < dspl; bi++)
                    {
                        int i = index()-origin;
                        cbuff1_a[bi] = interpolate0_sht(buf[i],buf[i+1],buf[i+2],buf[i+3]);
                        advance(dsp_phase_incr);
                    }
                    float d = 287440896.0f; // 32768 * 8772
                    pic::vector::vectdiv(&d,0,&cbuff1_a[0],1,&cbuff1_a[0],1,dspl);
                }

                pic::vector::vectmul(&cbuff1_a[0],1,&atten,0,&cbuff1_a[0],1,dspl);
            }

            void interpolate_stereo(const phase_t &dsp_phase_incr,unsigned dsp_start,unsigned dsp_end,float *dsp_buf_l,float *dsp_buf_r,float pan_l,float pan_r,piw::samplereader_t *reader,float atten)
            {
                uint32_t dspl = dsp_end - dsp_start;

                interpolate_calc(dsp_phase_incr,dspl,reader,atten);

                pic::vector::vectmul(&cbuff1_a[0],1,&pan_r,0,&cbuff2_a[0],1,dspl);
                pic::vector::vectmul(&cbuff1_a[0],1,&pan_l,0,&cbuff1_a[0],1,dspl);

                pic::vector::vectadd(&dsp_buf_l[dsp_start],1,&cbuff1_a[0],1,&dsp_buf_l[dsp_start],1,dspl);
                pic::vector::vectadd(&dsp_buf_r[dsp_start],1,&cbuff2_a[0],1,&dsp_buf_r[dsp_start],1,dspl);
            }

            void interpolate(const phase_t &dsp_phase_incr,unsigned dsp_start,unsigned dsp_end,float *dsp_buf,piw::samplereader_t *reader,float atten)
            {
                uint32_t dspl = dsp_end - dsp_start;

                interpolate_calc(dsp_phase_incr,dspl,reader,atten);

                pic::vector::vectadd(&dsp_buf[dsp_start],1,&cbuff1_a[0],1,&dsp_buf[dsp_start],1,dspl);
            }

            float interpolate0_flt(float x0, float x1, float x2, float x3)
            {
                interp_coeff_t *c = row();
                return (c->a0*x0 + c->a1*x1 + c->a2*x2 + c->a3*x3);
            }

            float interpolate0_sht(short x0, short x1, short x2, short x3)
            {
                interp_coeff2_t *c = row2();
                // due to the nature of the coeff lookup table, there can be no overflows when using long
                // (the negatives and positives are balanced in the entries of the lookup table)
                return float(c->a0*x0 + c->a1*x1 + c->a2*x2 + c->a3*x3);
            }

        private:
            union
            {
#ifdef PI_BIGENDIAN
                struct
                {
                    int32_t index;
                    uint32_t fract;
                } b32;
#else
                struct
                {
                    uint32_t fract;
                    int32_t index;
                } b32;
#endif
                long long b64;
            };

            struct interp_coeff_t
            {
                float a0, a1, a2, a3;
            };

            struct interp_coeff2_t
            {
                long a0, a1, a2, a3;
            };

            static interp_coeff_t interp_coeff[];
            static interp_coeff2_t interp_coeff2[];

            interp_coeff_t *row() const
            {
               int r = ((int)((b32.fract & 0xff000000) >> 24));
               return &interp_coeff[r];
            }

            interp_coeff2_t *row2() const
            {
               int r = ((int)((b32.fract & 0xff000000) >> 24));
               return &interp_coeff2[r];
            }

            piw::data_nb_t cbuff1;
            piw::data_nb_t cbuff2;
            float *cbuff1_a;
            float *cbuff2_a;

            /*
            void tablegen()
            {
                long mult=8772;
                for(int i = 0; i < 256; i++)
                {
                    interp_coeff_t v = interp_coeff[i];
                    std::cout << "{ " << (long)(v.a0*mult) << ", " << (long)(v.a1*mult) << ", " << (long)(v.a2*mult) << ", " << (long)(v.a3*mult) << " }," << std::endl;
                }
            }
            */
    };
}

#endif
