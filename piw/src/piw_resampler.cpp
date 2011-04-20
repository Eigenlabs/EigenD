
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

#include <piw/piw_resampler.h>
#include <lib_samplerate/lib_samplerate.h>
#include <picross/pic_log.h>
#include <string>

#define BUFFER_LENGTH 8192
#define AVG_LENGTH 512
#define WARMUP 40
#define THRESHOLD  200
#define QUALITY_MAX 4
#define CORRECTION_FACTOR 3.0
#define PLL_COEFF 0.8

struct piw::resampler_t::impl_t: pic::lckobject_t
{
    impl_t(const std::string &label, unsigned channels, unsigned quality): label_(label), channels_(channels), shorts_(0)
    {
        int err;
        src_state_ = src_new((quality<=QUALITY_MAX)?(QUALITY_MAX-quality):QUALITY_MAX,channels_,&err);
        reset();
    }

    void reset()
    {
        buffer_length_=0;
        buffer_avglen_=0;
        buffer_record_=0;
        out_counter_=0;
        memset(buffer_avg_,0,sizeof(buffer_avg_));
        src_reset(src_state_);
    }

    ~impl_t()
    {
        src_delete(src_state_);
    }

    void resampler_write_interleaved(const float *d, unsigned dl)
    {
        buffer_record_ += dl;

        if(channels_*(buffer_length_+dl) > BUFFER_LENGTH)
        {
            pic::logmsg() << label_ << " input buffer overflow";
            reset();
            return;
        }

        memcpy(&buffer_[buffer_length_*channels_],d,dl*sizeof(float)*channels_);
        buffer_length_ += dl;
    }
      
    unsigned resampler_read_interleaved(unsigned bs, float *outbuffer)
    {
        unsigned i = (out_counter_++) % AVG_LENGTH;
        buffer_avglen_ -= buffer_avg_[i];
        buffer_avg_[i] = buffer_record_;
        buffer_avglen_ += buffer_avg_[i];
        buffer_record_ = 0;

        int iwin = buffer_avglen_/std::min(out_counter_,(unsigned)AVG_LENGTH);

        if(out_counter_ <= WARMUP)
        {
            if(buffer_length_ > THRESHOLD)
            {
                memcpy(&buffer_[0],&buffer_[buffer_length_-THRESHOLD],THRESHOLD);
                buffer_length_ = THRESHOLD;
            }

            if(out_counter_==WARMUP)
            {
                pic::logmsg() << label_ << " warming up: iwin=" << iwin
                                   << " bs=" << bs << " buffer=" << buffer_length_
                                   << " ratio=" << ((double)bs)/((double)iwin);
            }

            error_ = 0;

            return 0;
        }

        if((int)buffer_length_<iwin)
        {
            //pic::logmsg() << label_ << " short buffer " << iwin << " " << buffer_length_;
            iwin = buffer_length_;
        }

        if(iwin==0)
        {
            pic::logmsg() << label_ << " empty window";
            return 0;
        }

        double error = ((double)(buffer_length_-iwin-THRESHOLD));
        error_ = (PLL_COEFF*error_)+((1.0-PLL_COEFF)*error);
        double correction = error_/((double)(CORRECTION_FACTOR*THRESHOLD));
        double ratio = ((double)bs)/(((double)iwin)+correction);
        int ibs = std::min(iwin+2,buffer_length_);

        SRC_DATA src_data;
        src_data.data_in = buffer_;
        src_data.data_out = outbuffer;
        src_data.input_frames = ibs;
        src_data.output_frames = bs;
        src_data.src_ratio = ratio;
        src_data.end_of_input = 0;

        int e;

        if((e=src_process(src_state_,&src_data)) != 0)
        {
            pic::logmsg() << label_ << " conversion error " << e;
            src_data.input_frames_used = std::min(iwin,ibs);
            src_data.output_frames_gen = 0;
        }

        buffer_length_-=src_data.input_frames_used;

        if(src_data.output_frames_gen != (int)bs)
        {
            shorts_++;
            //pic::logmsg() << label_ << " short output " << src_data.output_frames_gen;
        }

        if(out_counter_%1000 == 0)
        {
            pic::logmsg() << label_ << " buffer length " << buffer_length_
                          << " ratio " << ratio << " error " << error << "->" <<error_ << " correction " << correction
                          << " shorts " << shorts_
                          << " iwin " << iwin;
            shorts_ = 0;
        }

        if(buffer_length_>0)
        {
            memcpy(&buffer_[0],&buffer_[src_data.input_frames_used*channels_],buffer_length_*sizeof(float)*channels_);
        }

        return src_data.output_frames_gen;
    }

    std::string label_;
    unsigned channels_;
    unsigned buffer_avg_[AVG_LENGTH];
    float buffer_[BUFFER_LENGTH];
    int buffer_length_;
    SRC_STATE *src_state_;
    unsigned out_counter_;
    unsigned buffer_avglen_;
    unsigned buffer_record_;
    unsigned shorts_;
    double error_;
};

piw::resampler_t::resampler_t(const char *label, unsigned channels, unsigned quality)
{
    impl_.set(new impl_t(label,channels,quality));
}

void piw::resampler_t::set_quality(unsigned q)
{
    impl_t *old = impl_.current();
    impl_.set(new impl_t(old->label_,old->channels_,q));
    pic::logmsg() << old->label_ << " quality now " << q;
    delete old;
}

piw::resampler_t::~resampler_t()
{
    delete impl_.current();
}

void piw::resampler_t::reset()
{
    pic::flipflop_t<impl_t *>::guard_t g(impl_);
    g.value()->reset();
}

unsigned piw::resampler_t::resampler_read_interleaved(unsigned bs, float *outbuffer)
{
    pic::flipflop_t<impl_t *>::guard_t g(impl_);
    return g.value()->resampler_read_interleaved(bs,outbuffer);
}

void piw::resampler_t::resampler_write_interleaved(const float *d, unsigned dl)
{
    pic::flipflop_t<impl_t *>::guard_t g(impl_);
    g.value()->resampler_write_interleaved(d,dl);
}
