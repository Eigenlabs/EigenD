
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

#include <picross/pic_stl.h>
#include <picross/pic_float.h>

#include <piw/piw_address.h>
#include <piw/piw_sampler.h>
#include <piw/piw_phase.h>

#include <math.h>
#include <list>
#include <map>

static PIC_FASTCODE const float fadein__[128] = {
0.0000000000, 0.0001529714, 0.0006117919, 0.0013761808, 
0.0024456704, 0.0038196063, 0.0054971478, 0.0074772683, 
0.0097587564, 0.0123402160, 0.0152200676, 0.0183965490, 
0.0218677165, 0.0256314462, 0.0296854352, 0.0340272028, 
0.0386540925, 0.0435632730, 0.0487517405, 0.0542163202, 
0.0599536686, 0.0659602749, 0.0722324638, 0.0787663975, 
0.0855580779, 0.0926033493, 0.0998979008, 0.1074372689, 
0.1152168405, 0.1232318554, 0.1314774092, 0.1399484566, 
0.1486398144, 0.1575461643, 0.1666620568, 0.1759819139, 
0.1855000331, 0.1952105901, 0.2051076434, 0.2151851371, 
0.2254369048, 0.2358566737, 0.2464380681, 0.2571746133, 
0.2680597399, 0.2790867873, 0.2902490084, 0.3015395730, 
0.3129515727, 0.3244780246, 0.3361118759, 0.3478460079, 
0.3596732407, 0.3715863375, 0.3835780087, 0.3956409168, 
0.4077676808, 0.4199508804, 0.4321830608, 0.4444567375, 
0.4567644003, 0.4690985183, 0.4814515445, 0.4938159202, 
0.5061840798, 0.5185484555, 0.5309014817, 0.5432355997, 
0.5555432625, 0.5678169392, 0.5800491196, 0.5922323192, 
0.6043590832, 0.6164219913, 0.6284136625, 0.6403267593, 
0.6521539921, 0.6638881241, 0.6755219754, 0.6870484273, 
0.6984604270, 0.7097509916, 0.7209132127, 0.7319402601, 
0.7428253867, 0.7535619319, 0.7641433263, 0.7745630952, 
0.7848148629, 0.7948923566, 0.8047894099, 0.8144999669, 
0.8240180861, 0.8333379432, 0.8424538357, 0.8513601856, 
0.8600515434, 0.8685225908, 0.8767681446, 0.8847831595, 
0.8925627311, 0.9001020992, 0.9073966507, 0.9144419221, 
0.9212336025, 0.9277675362, 0.9340397251, 0.9400463314, 
0.9457836798, 0.9512482595, 0.9564367270, 0.9613459075, 
0.9659727972, 0.9703145648, 0.9743685538, 0.9781322835, 
0.9816034510, 0.9847799324, 0.9876597840, 0.9902412436, 
0.9925227317, 0.9945028522, 0.9961803937, 0.9975543296, 
0.9986238192, 0.9993882081, 0.9998470286, 1.0000000000, 
};

namespace
{
    struct xvoice_t: pic::atomic_counted_t, pic::lckobject_t
    {
        xvoice_t(const piw::sampleref_t &s, float pan): sample_(s), phase_(s->start(),0), done_(false), start_(s->start()), end_(s->end()), loopstart_(s->loopstart()), loopend_(s->loopend()), rootfreq_(s->rootfreq()), attenuation_(s->attenuation()), samplerate_(s->samplerate()), reader_(s)
        {
            pan_l_ = (pan+1.0)/2.0;
            pan_r_ = 1.0-pan_l_;
            recalc_freq(48000,440);
        }

        ~xvoice_t()
        {
        }

        void recalc_freq(float sr,float freq)
        {
            float rootfreq = rootfreq_*(sr/samplerate_);
            incr_ = freq/rootfreq;
        }

        bool dsp_write(float *dsp_buf_l, float *dsp_buf_r,unsigned len)
        {
            while(len > 32)
            {
                if(done_)
                    return true;

                dsp_write32(dsp_buf_l, dsp_buf_r, 32);
                dsp_buf_l += 32; dsp_buf_r +=32; len -= 32;
            }

            if(len)
            {
                if(done_)
                    return true;

                dsp_write32(dsp_buf_l, dsp_buf_r, len);
            }

            return done_;
        }

        void dsp_write32(float *dsp_buf_l, float *dsp_buf_r, unsigned len)
        {
            float incr = incr_;
            piw::phase_t dsp_phase_incr(incr_);

            if(loopend_)
            {
                unsigned end_in_buffer = phase_.steps(loopend_, incr);
                if(end_in_buffer >= len)
                {
                    unsigned dsp_start = 0;
                    unsigned dsp_end = len;
                    phase_.interpolate_stereo(dsp_phase_incr,dsp_start,dsp_end,dsp_buf_l,dsp_buf_r,pan_l_,pan_r_,&reader_,attenuation_);
                }
                else
                {
                    unsigned start = 0;
                    while(end_in_buffer < len)
                    {
                        unsigned dsp_start = start;
                        unsigned dsp_end = end_in_buffer;
                        phase_.interpolate_stereo(dsp_phase_incr,dsp_start,dsp_end,dsp_buf_l,dsp_buf_r,pan_l_,pan_r_,&reader_,attenuation_);
                        phase_ -= (loopend_-loopstart_);
                        start = end_in_buffer;
                        end_in_buffer += phase_.steps(loopend_, incr);
                    }

                    unsigned dsp_start = start;
                    unsigned dsp_end = len;
                    phase_.interpolate_stereo(dsp_phase_incr,dsp_start,dsp_end,dsp_buf_l,dsp_buf_r,pan_l_,pan_r_,&reader_,attenuation_);
                }
            }
            else
            {
                unsigned end_in_buffer = phase_.steps(end_, incr);
                if(end_in_buffer >= len)
                {
                    unsigned dsp_start = 0;
                    unsigned dsp_end = len;
                    phase_.interpolate_stereo(dsp_phase_incr,dsp_start,dsp_end,dsp_buf_l,dsp_buf_r,pan_l_,pan_r_,&reader_,attenuation_);
                }
                else
                {
                    unsigned dsp_start = 0;
                    unsigned dsp_end = end_in_buffer;
                    phase_.interpolate_stereo(dsp_phase_incr,dsp_start,dsp_end,dsp_buf_l,dsp_buf_r,pan_l_,pan_r_,&reader_,attenuation_);
                    done_ = true;
                }
            }
		}

        piw::sampleref_t sample_;
        piw::phase_t phase_;
        bool done_;
        unsigned start_, end_;
        unsigned loopstart_, loopend_;
        float rootfreq_;
        float attenuation_;
        float pan_l_,pan_r_;
        float incr_;
        float samplerate_;
        piw::samplereader_t reader_;
    };

    struct voiceimpl_t: piw::sampler_voice_t
    {
        voiceimpl_t(const piw::voiceref_t &z): samples_(0), channel_count_(z->zones.size()), attenuation_(0), fading_in_(0), fading_out_(127)
        {
            samples_= new pic::ref_t<xvoice_t>[z->zones.size()];

			pic::lcklist_t<piw::zoneref_t>::nbtype::const_iterator zi,zb,ze;

            zb = z->zones.begin();
            ze = z->zones.end();

            int i = 0;
            for(zi=zb;zi!=ze;zi++)
            {
                samples_[i++] = pic::ref(new xvoice_t((*zi)->c,(*zi)->p));
            }
        }

        ~voiceimpl_t()
        {
            delete [] samples_;
            samples_= 0;
        }

        void disable_fadein()
        {
            fading_in_=128;
            attenuation_=1.0;
        }

        bool fade_in(float *out0,float *out1, unsigned from, unsigned to)
        {
            unsigned len = to-from;
            PIC_ASSERT(len<=PLG_CLOCK_BUFFER_SIZE);
            float buffer0[PLG_CLOCK_BUFFER_SIZE];
            float buffer1[PLG_CLOCK_BUFFER_SIZE];
            bool done = true;

            memset(buffer0,0,sizeof(float)*len);
            memset(buffer1,0,sizeof(float)*len);

            for(int i=0; i< channel_count_; i++)
            {
                if(!samples_[i]->dsp_write(buffer0,buffer1, len))
                {
                    done=false;
                }
            }

            float a = fadein__[fading_in_];

            for(unsigned x=0;x<len;x++)
            {
                buffer0[x] *= a;
                buffer1[x] *= a;

                if(fading_in_<127)
                {
                    a = fadein__[++fading_in_];
                }
            }

            attenuation_=a;

            pic::vector::vectadd(&out0[from],1,buffer0,1,&out0[from],1,len);
            pic::vector::vectadd(&out1[from],1,buffer1,1,&out1[from],1,len);

            return done;
        }

        bool fade(float *out0, float *out1, unsigned from, unsigned to)
        {
            unsigned len = to-from;
            PIC_ASSERT(len<=PLG_CLOCK_BUFFER_SIZE);
            float buffer0[PLG_CLOCK_BUFFER_SIZE];
            float buffer1[PLG_CLOCK_BUFFER_SIZE];
            bool done = true;

            memset(buffer0,0,sizeof(float)*len);
            memset(buffer1,0,sizeof(float)*len);

            for(int i=0; i<channel_count_; i++)
            {
                if(!samples_[i]->dsp_write(buffer0,buffer1, len))
                {
                    done=false;
                }
            }

            float a = fadein__[fading_out_];

            for(unsigned x=0;x<len;x++)
            {
                buffer0[x] *= attenuation_*a;
                buffer1[x] *= attenuation_*a;

                if(fading_out_>0)
                {
                    a = fadein__[--fading_out_];
                }
            }

            pic::vector::vectadd(&out0[from],1,buffer0,1,&out0[from],1,len);
            pic::vector::vectadd(&out1[from],1,buffer1,1,&out1[from],1,len);

            return (fading_out_<=0) || done;
        }

        bool write(float *out0, float *out1, unsigned from, unsigned to)
        {
            if(fading_in_<127)
            {
                return fade_in(out0,out1,from,to);
            }

            unsigned len = to-from;
            bool done = true;

            for(int i=0; i<channel_count_; i++)
            {
                if(!samples_[i]->dsp_write(out0+from,out1+from, len))
                {
                    done=false;
                }
            }

            return done;
        }

        void recalc_freq(float sr, float freq)
        {
            for(int i=0; i<channel_count_; i++)
            {
                samples_[i]->recalc_freq(sr,freq);
            }
        }

        pic::ref_t<xvoice_t> *samples_;
        int channel_count_;
        float attenuation_;
        unsigned fading_in_,fading_out_;
    };
}

piw::sampler_voiceref_t piw::create_player(const piw::voiceref_t &zone)
{
    if(zone.isvalid())
    {
        return pic::ref(new voiceimpl_t(zone));
    }

    return piw::sampler_voiceref_t();
}

struct piw::sampler_t::impl_t
{
    impl_t(const piw::presetref_t &p): xpreset_(p)
    {
    }

    sampler_voiceref_t create_voice(float v, float f) const
    {
        piw::voiceref_t z = xpreset_->find_zone(v,f);

        if(!z.isvalid())
        {
            pic::logmsg() << "no applicable voice for velocity " << v << " frequency " << f;
        }

        return create_player(z);
    }

    piw::presetref_t xpreset_;
};

piw::sampler_t::sampler_t(const piw::presetref_t &p) : impl_(new impl_t(p)) {}
piw::sampler_t::~sampler_t() { delete impl_; }
piw::sampler_voiceref_t piw::sampler_t::create_voice(float v, float f) const { return impl_->create_voice(v,f); }
