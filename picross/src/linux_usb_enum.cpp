
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

#include <picross/pic_usb.h>
#include <picross/pic_log.h>
#include <picross/pic_thread.h>
#include <picross/pic_error.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <set>

struct pic::usbenumerator_t::impl_t: public pic::thread_t
{
    impl_t(unsigned short, unsigned short, const f_string_t &);
    ~impl_t();

    void thread_main();
    void thread_init();
    void thread_pass();
    void stop() { stop_=true; wait(); }
    static void attached(void *, unsigned short, unsigned short);

    unsigned short vendor_;
    unsigned short product_;
    f_string_t delegate_;
    pic::mutex_t lock_;
    bool stop_;

    std::set<std::pair<unsigned short, unsigned short> > devices_;
    std::set<std::pair<unsigned short, unsigned short> > working_;
};

static unsigned enumerate__(unsigned short vendor, unsigned short product, void (*callback)(void *, unsigned short, unsigned short), void *arg)
{
    FILE *fp;
    char buffer[80];
    int len,ch;
    int state=0;
    unsigned short v,p,b,d;
    unsigned count=0;

    if(!(fp=fopen("/sys/kernel/debug/usb/devices","r")))
    {
        pic::msg() << "can't open /sys/kernel/debug/usb/devices" << pic::log;
        return 0;
    }

    while(fgets(buffer,sizeof(buffer),fp))
    {
        if((len=strlen(buffer))==0)
        {
            continue;
        }

        if(buffer[len-1]!='\n')
        {
            while((ch=fgetc(fp))!=EOF && (ch!='\n')) ;
        }
        else
        {
            buffer[len-1]=0;
        }

        if(state==0 && buffer[0]=='T')
        {
            /* T:  Bus=03 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=480 MxCh= 8 */
            if(sscanf(buffer,"T:  Bus=%hu Lev=%*d Prnt=%*d Port=%*d Cnt=%*d Dev#=%hu",&b,&d)==2)
            {
                state=1;
            }
        }

        if(state==1 && buffer[0]=='P')
        {
            /* P:  Vendor=0000 ProdID=0000 Rev= 2.06 */
            if(sscanf(buffer,"P:  Vendor=%hx ProdID=%hx",&v,&p)==2)
            {
                state=0;

                if(vendor==v && product==p)
                {
                    try
                    {
                        callback(arg,b,d);
                    }
                    CATCHLOG()

                    count++;
                }
            }
        }
    }

    fclose(fp);
    return count;
}

static void call_delegate__(const pic::f_string_t &delegate, unsigned short b, unsigned short d)
{
    char path[80];
    sprintf(path,"/dev/bus/usb/%03hu/%03hu",b,d);

    try
    {
        delegate(path);
    }
    CATCHLOG()
}

pic::usbenumerator_t::impl_t::impl_t(unsigned short v, unsigned short p, const f_string_t &d): pic::thread_t(0), vendor_(v), product_(p), delegate_(d)
{
}

pic::usbenumerator_t::impl_t::~impl_t()
{
    stop();
}

void pic::usbenumerator_t::impl_t::attached(void *e_, unsigned short b, unsigned short d)
{
    pic::usbenumerator_t::impl_t *self = (pic::usbenumerator_t::impl_t *)e_;
    std::pair<unsigned short, unsigned short> p = std::make_pair(b,d);

    self->working_.insert(p);

    if(self->devices_.find(p) == self->devices_.end())
    {
        pic::mutex_t::guard_t g(self->lock_);
        call_delegate__(self->delegate_,b,d);
    }
}

void pic::usbenumerator_t::impl_t::thread_pass()
{
    working_.clear();
    enumerate__(vendor_,product_,attached,this);
    devices_=working_;
}

void pic::usbenumerator_t::impl_t::thread_init()
{
    stop_=false;
}

void pic::usbenumerator_t::impl_t::thread_main()
{
    while(!stop_)
    {
        thread_pass();
        sleep(1);
    }
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &d, const f_string_t &gone)
{
    impl_ = new impl_t(v,p,d);
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &d)
{
    impl_ = new impl_t(v,p,d);
}

pic::usbenumerator_t::~usbenumerator_t()
{
    delete impl_;
}

void pic::usbenumerator_t::start()
{
    impl_->run();
}

void pic::usbenumerator_t::stop()
{
    impl_->stop();
}

static void call_delegate2__(void *arg, unsigned short b, unsigned short d)
{
    call_delegate__(*(pic::f_string_t *)arg,b,d);
}

unsigned pic::usbenumerator_t::enumerate(unsigned short vendor, unsigned short product, const f_string_t &del)
{
    return enumerate__(vendor,product,call_delegate2__,(void *)&del);
}

int pic::usbenumerator_t::gc_traverse(void *v,void *a) const
{
    return impl_->delegate_.gc_traverse(v,a);
}

int pic::usbenumerator_t::gc_clear()
{
    pic::mutex_t::guard_t g(impl_->lock_);
    return impl_->delegate_.gc_clear();
}
