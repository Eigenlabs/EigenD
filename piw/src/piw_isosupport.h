
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

#ifndef __PIW_ISOSUPPORT__
#define __PIW_ISOSUPPORT__

#include <map>
#include <picross/pic_float.h>

//#define ISO_DEBUG 1

namespace piw
{
    /* B and A contain the numerator and denominator coefficients.
     * 
     * A(0) (gain) is assumed to be unity (thus array A is one smaller than B)
     * Arrays A and B are stored in reverse order ("oldest" first)
     */
    template <unsigned NP, float *B, float *A>
    struct iir_t
    {
        static float filter(const float *in, const float *out)
        {
            return pic::vector::dotprod(B, 1, in-NP, 1, NP+1) - pic::vector::dotprod(A, 1, out-NP, 1, NP);
        }
    };

    // filter stop edge 0.2*Fs, pass band edge 0.1*Fs,
    // pass band ripple 0.5dB
    // stop band attenuation 60dB (too low?)
    // chebyshev 6-pole, Wc=0.2
    float cheb6_b[7] = { 6.2839e-05,   3.7703e-04,   9.4258e-04,   1.2568e-03,   9.4258e-04,   3.7703e-04,   6.2839e-05 };
    float cheb6_a[6] = { 0.48336, -2.94382, 7.77932, -11.42125, 9.84167, -4.73501  };

    typedef iir_t<6,cheb6_b,cheb6_a> cheb6_t;

    template<unsigned SAMPLES,unsigned HISTORY>
    class buffer_t
    {
        public:
            buffer_t()
            {
                reset(0.0);
            }

            void reserve(unsigned len)
            {
                unsigned end = index_+len;
                if(end >= SAMPLES)
                {
                    unsigned from = index_-HISTORY;
                    memcpy(buf_, buf_+from, HISTORY*sizeof(float));
                    memset(buf_+HISTORY, 0, SAMPLES*sizeof(float));
                    index_ = HISTORY;
                }
            }

            void reset(float f)
            {
                for(unsigned i=0; i<(HISTORY+SAMPLES); i++)
                    buf_[i] = f;
                index_ = HISTORY;
                readable_ = 0;
            }

            float *write(unsigned n)
            {
#ifdef ISO_DEBUG
                if(readable_!=0)
                {
                    pic::msg() << "bad readable want 0 got " << readable_ << pic::log;
                }
#endif
                readable_ = n;
                reserve(n);
                return buf_+index_;
            }

            const float *read(unsigned n)
            {
#ifdef ISO_DEBUG
                if(readable_!=n)
                {
                    pic::msg() << "bad readable want " << n << " got " << readable_ << pic::log;
                }
#endif
                readable_ = 0;
                const float *f = buf_+index_;
                index_ += n;
                return f;
            }

        private:
            float buf_[SAMPLES+HISTORY];
            unsigned index_;
            unsigned readable_;
    };

    template <class UPSTREAM, class FILTER, unsigned SAMPLES, unsigned HISTORY>
    class isofilter_t
    {
        public:
            void process(unsigned n, unsigned long long t)
            {
                upstream_.process(n,t);
                do_filter(n);
            }

            void reset(float v)
            {
                upstream_.reset(v);
                buffer_.reset(v);
            }

            void do_filter(unsigned n)
            {
                const float *in = upstream_.read(n);
                float *out = buffer_.write(n);

                for(unsigned i=0; i<n; ++i)
                {
                    *out = FILTER::filter(in, out);
                    ++in;
                    ++out;
                }
            }

            const float *read(unsigned n)
            {
                return buffer_.read(n);
            }

            void input(const piw::data_nb_t &d,float u,float l,float z)
            {
                upstream_.input(d,u,l,z);
            }

        private:
            buffer_t<SAMPLES,HISTORY> buffer_;
            UPSTREAM upstream_;

    };

    template<class UPSTREAM, unsigned STRIDE, unsigned SAMPLES, unsigned HISTORY>
    class upsampler_t
    {
        public:
            void process(unsigned n, unsigned long long t)
            {
                upstream_.process(n/STRIDE,t);
                do_up(n/STRIDE);
            }

            void reset(float v)
            {
                upstream_.reset(v);
                buffer_.reset(v);
            }
            
            void do_up(unsigned n)
            {
                const float *in = upstream_.read(n);
                float *out = buffer_.write(n*STRIDE);

                for(unsigned i=0; i<n; ++i)
                {
                    *out = (*in)*STRIDE;
                    ++in;
                    out += STRIDE;
                }
            }

            const float *read(unsigned n)
            {
                return buffer_.read(n);
            }

            void input(const piw::data_nb_t &d,float u,float l,float z)
            {
                upstream_.input(d,u,l,z);
            }

        private:
            buffer_t<SAMPLES,HISTORY> buffer_;
            UPSTREAM upstream_;
    };

    template<class UPSTREAM, unsigned STRIDE, unsigned SAMPLES, unsigned HISTORY>
    class linear_interp_t
    {
        public:
            linear_interp_t() : current_(0.0), fstride_(STRIDE)
            {
            }

            void process(unsigned n, unsigned long long t)
            {
                upstream_.process(n/STRIDE,t);
                do_linear(n/STRIDE,n%STRIDE);
            }

            void reset(float v)
            {
                upstream_.reset(v);
                buffer_.reset(v);
                current_ = v;
            }
            
            void do_linear(unsigned n,unsigned r)
            {
                const float *in = upstream_.read(n);
                float *out = buffer_.write(n*STRIDE+r);
                float slope = 0;

                for(unsigned i=0; i<n; ++i)
                {
                    slope = (*in-current_)/fstride_;
                    for(unsigned j=0; j<STRIDE; ++j)
                    {
                        *out++ = current_+j*slope;
                    }
                    current_ = *in;
                    ++in;
                }

                for(unsigned j=0;j<r;j++)
                {
                    *out++ = current_+j*slope;
                }

            }

            const float *read(unsigned n)
            {
                return buffer_.read(n);
            }

            void input(const piw::data_nb_t &d,float u,float l,float z)
            {
                upstream_.input(d,u,l,z);
            }

        private:
            buffer_t<SAMPLES,HISTORY> buffer_;
            UPSTREAM upstream_;
            float current_;
            float fstride_;
    };

    template<unsigned long FREQ /* in 1/1000 Hz */, unsigned SAMPLES, unsigned HISTORY>
    class zerohold_t
    {
        public:
            zerohold_t() : current_(0.0)
            {
            }

            void process(unsigned n, unsigned long long t)
            {
                do_zerohold(n,t);
            }

            void reset(float v)
            {
                buffer_.reset(v);
                input_.clear();
                current_ = v;
            }

            void do_zerohold(unsigned n, unsigned long long tend)
            {
                unsigned long long bt = n*ST;
                unsigned long long tstart = tend-bt, t=tstart;

                std::map<unsigned long long, float>::iterator i,b,e;
                b = input_.begin();
                e = input_.upper_bound(tend);

                float *out = buffer_.write(n);

                for(i = b; i != e; ++i)
                {
                    unsigned samples = (i->first-t)/ST;
                    samples = std::min(samples,n);
                    n -= samples;

                    for(; samples > 0; --samples)
                    {
                        *out++ = current_;
                    }

                    current_ = i->second;
                    t = i->first;
                }

                for(; n > 0; --n)
                {
                    *out++ = current_;
                }

                input_.erase(b, e);
            }

            void input(const piw::data_nb_t &d, float ub, float lb, float z)
            {
                float r = d.as_renorm(lb,ub,z);
                input_.insert(std::make_pair(d.time(),r));
            }

            const float *read(unsigned n)
            {
                return buffer_.read(n);
            }

        private:
            static const unsigned long long ST = 1000000000/FREQ;

            buffer_t<SAMPLES,HISTORY> buffer_;
            std::map<unsigned long long, float> input_;

            float current_;
    };
}

#endif

