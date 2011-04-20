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

#include <picross/pic_config.h>
#ifndef PI_WINDOWS
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define O_BINARY 0
#else
#include <io.h>
typedef int ssize_t;
#endif

#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include <picross/pic_log.h>
#include "sng_file.h"

#define BLK_SHEADER    2
#define BLK_LHEADER    16
#define BLK_CHKPOINT   0
#define BLK_SIGNATURE  2
#define REC_HEADER     2
#define REC_SIZE       0

#define BLKSIZE_OLD    8192
#define BLKSIZE_NEW    65536

namespace
{
    struct buffer_t
    {
        buffer_t(): ptr_(0)
        {
        }

        void alloc(unsigned long size)
        {
            if(ptr_)
            {
                free(ptr_);
                ptr_ = 0;
            }

            ptr_ = (unsigned char *)malloc(size);
            PIC_ASSERT(ptr_);
        }

        ~buffer_t()
        {
            free(ptr_);
        }

        unsigned char *ptr_;
    };

    struct xfile_t: public pi::state::file_t
    {
        void seek__(unsigned long blk)
        {
            off_t wnt = blk*blk_size_;
            off_t act = lseek(fd_,wnt,SEEK_SET);
            PIC_ASSERT(wnt==act);
        }

        static pi::state::fileref_t create(const char *filename, bool writeable) { return pic::ref(new xfile_t(filename, writeable)); }

        std::string name()
        {
            return name_;
        }

        void set_blocksize()
        {
            unsigned long size = lseek(fd_,0,SEEK_END);
            blk_size_ = BLKSIZE_NEW;
            newformat_ = true;

            seek__(0);

            if(size<BLK_LHEADER)
            {
                return;
            }

            unsigned char sig[BLK_LHEADER];
            blk_size_ = BLKSIZE_OLD;
            newformat_ = false;

            if(read(fd_,sig,BLK_LHEADER)!=BLK_LHEADER)
            {
                return;
            }

            if(sig[2]==0xbe && sig[3]==0xca)
            {
                blk_size_ = BLKSIZE_NEW;
                newformat_ = true;
            }
        }

        xfile_t(const char *filename,bool writeable)
        {
            writeable_ = writeable;
            name_ = filename;

            if((fd_=::open(filename,writeable?(O_RDWR|O_CREAT|O_BINARY):(O_RDONLY|O_BINARY),0666))<0)
            {
                pic::msg() << "Can't open " << filename << pic::hurl;
            }

            unsigned long size = lseek(fd_,0,SEEK_END);
            set_blocksize();


            rbuffer_.alloc(blk_size_);
            wbuffer_.alloc(blk_size_);

            cpos_ = 0;
            wblk_ = size/blk_size_;

            while(wblk_>0)
            {
                seek__(wblk_-1);
                PIC_ASSERT(read(fd_,wbuffer_.ptr_,blk_size_)==(ssize_t)blk_size_);
                
                unsigned coffset = (wbuffer_.ptr_[BLK_CHKPOINT]<<8)+(wbuffer_.ptr_[BLK_CHKPOINT+1]);

                if(coffset > 0)
                {
                    cpos_ = ((wblk_-1)*blk_size_)+coffset;
                    break;
                }

                wblk_--;
            }

            rblk_ = ~0UL;
            wbuffer_.ptr_[BLK_CHKPOINT] = 0;
            wbuffer_.ptr_[BLK_CHKPOINT+1] = 0;

            if(wblk_)
            {
                woffset_ = BLK_SHEADER;
            }
            else
            {
                woffset_ = BLK_LHEADER;
                wbuffer_.ptr_[BLK_SIGNATURE] = 0xbe;
                wbuffer_.ptr_[BLK_SIGNATURE+1] = 0xca;
            }

            pic::logmsg() << "opened " << filename << " start block: " << wblk_ << " chkpoint: " << cpos_ << " blksize: " << blk_size_;
        }

        ~xfile_t()
        {
            close();
        }

        void flush()
        {
            if(writeable())
            {
                if(woffset_>BLK_SHEADER)
                {
                    seek__(wblk_);
                    PIC_ASSERT(write(fd_,wbuffer_.ptr_,blk_size_)==(ssize_t)blk_size_);

                    wblk_++;

                    wbuffer_.ptr_[BLK_CHKPOINT] = 0;
                    wbuffer_.ptr_[BLK_CHKPOINT+1] = 0;
                    woffset_ = BLK_SHEADER;
                }
            }
        }

        unsigned char *write_payload(unsigned size, unsigned long *position, bool checkpoint)
        {
            PIC_ASSERT(writeable());
            PIC_ASSERT(size+REC_HEADER<=blk_size_-BLK_LHEADER);

            if(size+REC_HEADER+woffset_ > blk_size_)
            {
                flush();
            }

            *position = (blk_size_*wblk_)+woffset_;
            unsigned char *record = &wbuffer_.ptr_[woffset_];

            record[REC_SIZE] = (size>>8)&0xff;
            record[REC_SIZE+1] = (size)&0xff;

            if(checkpoint)
            {
                cpos_ = *position;
                wbuffer_.ptr_[BLK_CHKPOINT] = (woffset_>>8)&0xff;
                wbuffer_.ptr_[BLK_CHKPOINT+1] = woffset_&0xff;
            }

            woffset_+=size+REC_HEADER;
            return &record[REC_HEADER];
        }

        const unsigned char *read_payload(unsigned long offset, unsigned *size)
        {
            unsigned long blk = offset/blk_size_;
            unsigned long ind = offset%blk_size_;

            PIC_ASSERT(isopen());
            PIC_ASSERT(blk<=wblk_);

            if(blk==0 && newformat_)
            {
                PIC_ASSERT(ind>=BLK_LHEADER);
            }
            else
            {
                PIC_ASSERT(ind>=BLK_SHEADER);
            }

            if(blk == wblk_)
            {
                *size = (wbuffer_.ptr_[ind]<<8)|(wbuffer_.ptr_[ind+1]);
                return &wbuffer_.ptr_[ind+REC_HEADER];
            }

            if(blk != rblk_)
            {
                seek__(blk);
                rblk_ = ~0UL;
                PIC_ASSERT(read(fd_,rbuffer_.ptr_,blk_size_)==(ssize_t)blk_size_);
                rblk_ = blk;
            }

            *size = (rbuffer_.ptr_[ind]<<8)|(rbuffer_.ptr_[ind+1]);
            return &rbuffer_.ptr_[ind+REC_HEADER];
        }

        std::string read_payload_string(unsigned long offset)
        {
            const unsigned char *buffer;
            unsigned size;

            buffer=read_payload(offset,&size);
            return std::string((const char *)buffer,size);
        }

        unsigned long write_payload_string(const std::string &data, bool checkpoint)
        {
            unsigned char *buffer;
            unsigned long position;

            buffer = write_payload(data.length(),&position,checkpoint);
            memcpy(buffer,data.c_str(),data.length());

            return position;
        }

        unsigned long checkpoint()
        {
            PIC_ASSERT(isopen());
            return cpos_;
        }

        bool isopen()
        {
            return fd_>=0;
        }

        void close()
        {
            if(isopen())
            {
                flush();
                ::close(fd_);
                fd_=-1;
            }
        }

        bool writeable()
        {
            return isopen() && writeable_;
        }

        int fd_;
        std::string name_;
        buffer_t rbuffer_;
        buffer_t wbuffer_;
        unsigned long rblk_;
        unsigned long wblk_;
        unsigned long woffset_;
        unsigned long cpos_;
        bool writeable_;
        unsigned long blk_size_;
        bool newformat_;
    };
};

pi::state::fileref_t pi::state::open_file(const char *filename,bool writeable)
{
    return xfile_t::create(filename,writeable);
}
