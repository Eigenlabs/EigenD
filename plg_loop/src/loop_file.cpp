
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

#include <sys/types.h>

#include <fcntl.h>
#include <picross/pic_log.h>
#include <picross/pic_nocopy.h>
#include <picross/pic_endian.h>
#include <picross/pic_config.h>
#include <picross/pic_resources.h>

#include <string.h>
#include "loop_file.h"


#define _PACK(c0,c1,c2,c3) ((c0<<24)+(c1<<16)+(c2<<8)+c3)

#ifdef PI_WINDOWS
#define LOOP_OPENFLAGS (O_RDONLY|O_BINARY)
#else
#define LOOP_OPENFLAGS (O_RDONLY)
#endif

namespace loop
{
    static const uint32_t FORM = _PACK('F','O','R','M');
    static const uint32_t AIFF = _PACK('A','I','F','F');
    static const uint32_t AIFC = _PACK('A','I','F','C');
    static const uint32_t COMM = _PACK('C','O','M','M');
    static const uint32_t SSND = _PACK('S','S','N','D');
    static const uint32_t basc = _PACK('b','a','s','c');
    static const uint32_t trns = _PACK('t','r','n','s');
    static const uint32_t cate = _PACK('c','a','t','e');

    struct aiff_reader_t : pic::nocopy_t
    {
        std::string cname()
        {
            std::string ret;
            ret += *((char *)&chunk_);
            ret += *(((char *)&chunk_)+1);
            ret += *(((char *)&chunk_)+2);
            ret += *(((char *)&chunk_)+3);
            return ret;
        }

        aiff_reader_t(const char *n, loop::loopraw_t *l, bool justmeta) : name_(n), loop_(l), raw_(0)
        {
            if(!justmeta)
                pic::msg() << "loading loop from " << name_ << pic::log;

            fd_ = pic::open(name_, LOOP_OPENFLAGS);
            if(fd_ < 0) error("open");

            try
            {
                read_header();
                if(chunk_ != FORM) error("form decode");

                switch(read_ulong())
                {
                    case AIFF: aifc_ = false; break;
                    case AIFC: aifc_ = true; break;
                    default: error("file type");
                }

                while(read_header())
                {
                    switch(chunk_)
                    {
                        case COMM: decode_COMM(); break;
                        case SSND: if(!justmeta) decode_SSND(); break;
                        case basc: decode_basc(); break;
                        case trns: decode_trns(); break;
                        case cate: decode_cate(); break;
                    }

                    skip(remain_);
                }

                close(fd_);
            }
            catch(...)
            {
                delete[] raw_;
                close(fd_);
                throw;
            }

            if(!justmeta)
            {
                loop_->decode_samples(raw_);
                pic::msg() << name_ << " loop has " << loop_->samplecount_ << " frames, " << 
                    loop_->numchannels_ << " channels, " << loop_->numtransients_ << " transients, " << 
                    loop_->beats_ << " beats, in " << loop_->numer_ << "/" << loop_->denom_ <<
                    " time, sample rate is " << loop_->srate_ << " bit depth " << loop_->width_ << pic::log;
            }
        }

        ~aiff_reader_t()
        {
            delete[] raw_;
        }

        bool read_header()
        {
            if(read(fd_, &chunk_, 4) != 4) return false;
            if(read(fd_, &remain_, 4) != 4) return false;
            remain_ = pic_ntohl(remain_);
            chunk_ = pic_ntohl(chunk_);
            return true;
        }

        void decode_COMM()
        {
            loop_->numchannels_ = read_ushort();
            loop_->samplecount_ = read_ulong();
            loop_->width_ = read_ushort();
            loop_->srate_ = read_extended();
        }

        void decode_SSND()
        {
            uint32_t offset = read_ulong();
            skip(4+offset); // blksize
            raw_ = new char[remain_];
            read_raw(raw_,remain_);
        }

        void decode_basc()
        {
            skip(4); // version
            loop_->beats_ = read_ulong();
            loop_->note_ = read_ushort();
            loop_->scale_ = read_ushort();
            loop_->numer_ = read_ushort();
            loop_->denom_ = read_ushort();
            loop_->looping_ = read_ushort();
        }

        void decode_trns()
        {
            skip(72);

            uint32_t n = read_ulong();
            loop_->numtransients_ = n;
            loop_->transients_ = new uint32_t[n];
            for(uint32_t i = 0; i < n; ++i)
            {
                skip(4);
                loop_->transients_[i] = read_ulong();
                skip(16);
            }
        }

        void decode_cate()
        {
            skip(4);

            char str[50];
            memset(str,0,50); read_raw(str,50); if(str[0]) loop_->tags_.push_back(str);
            memset(str,0,50); read_raw(str,50); if(str[0]) loop_->tags_.push_back(str);
            memset(str,0,50); read_raw(str,50); if(str[0]) loop_->tags_.push_back(str);

            skip(64);

            uint32_t n = read_ulong();

            for(; n>0; --n)
            {
                memset(str,0,50); read_raw(str,50); if(str[0]) loop_->tags_.push_back(str);
            }
        }

        uint32_t read_ulong()
        {
            char b[4]; read_raw(b,4);
            return pic_ntohl(*(uint32_t *)b);
        }

        uint16_t read_ushort()
        {
            char b[2]; read_raw(b,2);
            return pic_ntohs(*(uint16_t *)b);
        }

        int16_t read_short()
        {
            char b[2]; read_raw(b,2);
            return (int16_t)pic_ntohs(*(uint16_t *)b);
        }

        uint8_t read_byte()
        {
            char b; read_raw(&b,1);
            return (uint8_t)b;
        }

        uint64_t read_extended()
        {
            short exp = read_short();
            if(exp < 0) error("unsupported conversion");
            exp = (63 - exp) + 0x3fff;
            uint64_t mh = read_ulong(), ml = read_ulong();
            // all sorts of ieee weirdness ommitted
            return ((mh<<32)+ml)>>exp;
        }

        void read_raw(char *ptr, uint32_t size)
        {
            if(remain_ < size) error("read");
            if(read(fd_, ptr, size) != (long)size) error("read");
            remain_ -= size;
        }

        void error(const char *what)
        {
            pic::msg() << "error: " << name_ << ": " << what << pic::hurl;
        }

        void skip(uint32_t l)
        {
            if(l > remain_) error("skip");
            if(lseek(fd_, l, SEEK_CUR) < 0) error("skip");
            remain_ -= l;
        }

        const char *name_;
        loop::loopraw_t *loop_;
        char *raw_;
        int fd_;
        bool aifc_;
        uint32_t chunk_;
        uint32_t remain_;
    };
}

static float __read16(const char **ptr)
{
    uint8_t *raw = (uint8_t *)*ptr;
    uint8_t b0 = *raw++;
    uint8_t b1 = *raw++;

    int16_t s = (b0<<8) | (b1);

    *ptr += 2;

    return (float)s/(float)((1<<15)-1);
}

static float __read24(const char **ptr)
{
    uint8_t *raw = (uint8_t *)*ptr;
    uint8_t b0 = *raw++;
    uint8_t b1 = *raw++;
    uint8_t b2 = *raw++;

    int32_t s = (b0<<24) | (b1<<16) | (b2<<8) | ((b0&0x80) ? 0xff:0);

    *ptr += 3;

    return (float)s/(float)((1U<<31)-1);
}

namespace loop
{
    loopraw_t::loopraw_t() :
            width_(0),
            samplecount_(0),
            srate_(0),
            beats_(8),
            note_(0),
            scale_(0),
            numer_(4),
            denom_(4),
            looping_(0),
            samples_(0),
            numchannels_(0),
            numtransients_(0),
            transients_(0)
    {
    }

    void loopraw_t::decode_samples(const char *raw)
    {
        switch(width_)
        {
            case 16:
                read(raw,__read16);
                break;
            case 24:
                read(raw,__read24);
                break;
            default:
                pic::msg() << "unsupported sample fmt (bit width " << width_ << ")" << pic::hurl;
        }
    }

    void loopraw_t::read(const char *raw, float (*reader)(const char **))
    {
        PIC_ASSERT(!samples_);
        samples_ = new float[samplecount_*numchannels_];
        for(unsigned i=0; i<samplecount_*numchannels_; ++i)
        {
            samples_[i] = reader(&raw);
        }
    }

    loopref_t read_aiff(const char *name,bool justmeta)
    {
        loopref_t l = pic::ref(new loopraw_t);
        aiff_reader_t r(name,l.ptr(),justmeta);
        return l;
    }
}
