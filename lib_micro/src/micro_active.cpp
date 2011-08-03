
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

#include <stdlib.h>

#include <picross/pic_config.h>
#include <picross/pic_log.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_endian.h>

#include <lib_micro/micro_active.h>
#include <lib_micro/micro_usb.h>

#define KEYS 17

struct micro::active_t::impl_t: pic::usbdevice_t::iso_in_pipe_t, pic::usbdevice_t::power_t, pic::usbdevice_t, virtual pic::lckobject_t
{
    impl_t(const char *, micro::active_t::delegate_t *);
    ~impl_t() { stop(); }

    void in_pipe_data(const unsigned char *frame, unsigned length, unsigned long long hf, unsigned long long ht,unsigned long long pt);
    void pipe_died(unsigned reason);
    void pipe_error(unsigned long long fnum, int err);
    void start();
    void stop();

    void on_suspending()
    {
        printf("keyboard suspending\n");
    }

    void on_waking()
    {
        printf("keyboard waking\n");
        control_out(TYPE_VENDOR,BCTMICRO_USBCOMMAND_START,0,0,0,0);
        resync_=true;
    }

    static void decode_cooked(void *ctx,unsigned long long ts,int tp, int id,unsigned a,unsigned p,int r,int y);
    static void decode_raw(void *ctx,int resync,const pico_rawkbd_t *);

    micro::active_t::delegate_t *handler_;
    bool raw_;
    pico_decoder_t decoder_;
    bool resync_;
};

micro::active_t::impl_t::impl_t(const char *name, micro::active_t::delegate_t *del): usbdevice_t::iso_in_pipe_t(BCTMICRO_USBENDPOINT_SENSOR_NAME,BCTMICRO_USBENDPOINT_SENSOR_SIZE), usbdevice_t(name,BCTMICRO_INTERFACE), handler_(del), raw_(false), resync_(false)
{
    add_iso_in(this);
    set_power_delegate(this);
    pico_decoder_create(&decoder_,PICO_DECODER_MICRO);
}

void micro::active_t::impl_t::start()
{
    start_pipes();
    control_out(TYPE_VENDOR,BCTMICRO_USBCOMMAND_START,0,0,0,0);
}

void micro::active_t::impl_t::stop()
{
    try
    {
        control_out(TYPE_VENDOR,BCTMICRO_USBCOMMAND_STOP,0,0,0,0);
    }
    catch(...)
    {
        pic::logmsg() << "device shutdown failed";
    }

    stop_pipes();
}

micro::active_t::active_t(const char *name, micro::active_t::delegate_t *handler)
{
    _impl = new impl_t(name,handler);
}

micro::active_t::~active_t()
{
    delete _impl;
}

void micro::active_t::start()
{
    _impl->start();
}

void micro::active_t::stop()
{
    _impl->stop();
}

bool micro::active_t::poll(unsigned long long t)
{
    return _impl->poll_pipe(t);
}

const char *micro::active_t::get_name()
{
    return "ukbd";
}

void micro::active_t::set_led(unsigned key, unsigned colour)
{
    _impl->control(TYPE_VENDOR,BCTMICRO_USBCOMMAND_SETLED,colour,key);
}

void micro::active_t::impl_t::decode_cooked(void *self,unsigned long long ts,int tp, int id,unsigned a,unsigned p,int r,int y)
{
    impl_t *impl = (impl_t *)self;

    switch(tp)
    {
        case PICO_DECODER_KEY:
            impl->handler_->kbd_key(ts,id,a,p,r,y);
            break;
        case PICO_DECODER_BREATH:
            impl->handler_->kbd_breath(ts,p);
            break;
        case PICO_DECODER_STRIP:
            impl->handler_->kbd_strip(ts,p);
            break;
    }
}

void micro::active_t::impl_t::decode_raw(void *self, int resync,const pico_rawkbd_t *rawkeys)
{
    impl_t *impl = (impl_t *)self;
    impl->handler_->kbd_raw(resync,*rawkeys);
}

void micro::active_t::impl_t::in_pipe_data(const unsigned char *frame, unsigned length, unsigned long long hf, unsigned long long ht, unsigned long long pt)
{
    if(raw_)
    {
        pico_decoder_raw(&decoder_,resync_?1:0,frame,length,ht,decode_raw,this);
    }
    else
    {
        pico_decoder_cooked(&decoder_,resync_?1:0,frame,length,ht,decode_cooked,this);
    }
}

void micro::active_t::impl_t::pipe_error(unsigned long long fnum, int err)
{
    pic::msg() << "usb error in frame " << fnum << " err=" << std::hex << err << pic::log;
    resync_=true;
}

void micro::active_t::impl_t::pipe_died(unsigned reason)
{
    pic::logmsg() << "micro::active pipe died";
    handler_->kbd_dead();
}

std::string micro::active_t::debug()
{
    return _impl->control_in(0x80|0x40,0xc0,0,0,6);
}

static void __write16(unsigned char *raw, unsigned i, unsigned short val)
{
    raw[i*2] = (val>>8)&0x00ff;
    raw[i*2+1] = (val>>0)&0x00ff;
}

void micro::active_t::set_calibration(unsigned key, unsigned corner, unsigned short min, unsigned short max, const unsigned short *table)
{
    unsigned char raw[2*BCTMICRO_CALTABLE_SIZE];

    __write16(raw,BCTMICRO_CALTABLE_MIN,min);
    __write16(raw,BCTMICRO_CALTABLE_MAX,max);

    for(unsigned i=0;i<BCTMICRO_CALTABLE_POINTS;i++)
    {
        __write16(raw,BCTMICRO_CALTABLE_DATA+i,table[i]);
    }

    _impl->control_out(TYPE_VENDOR,BCTMICRO_USBCOMMAND_CALDATA,corner,key,raw,2*BCTMICRO_CALTABLE_SIZE);
}

void micro::active_t::write_calibration()
{
    _impl->control(TYPE_VENDOR,BCTMICRO_USBCOMMAND_CALWRITE,0,0);
}

void micro::active_t::clear_calibration()
{
    _impl->control(TYPE_VENDOR,BCTMICRO_USBCOMMAND_CALCLEAR,0,0);
}

unsigned micro::active_t::get_temperature()
{
    return debug()[0];
}

void micro::active_t::load_calibration_from_device()
{
    pic::logmsg() << "loading calibration from device";

    unsigned short min,max,row[BCTMICRO_CALTABLE_POINTS+2];
    row[0] = 0;
    row[BCTMICRO_CALTABLE_POINTS+1] = 4095;

    try
    {
        for(unsigned k=0; k<17; ++k)
        {
            for(unsigned c=0; c<4; ++c)
            {
                if(get_calibration(k,c,&min,&max,row+1))
                {
                    pico_decoder_cal(&_impl->decoder_,k,c,min,max,BCTMICRO_CALTABLE_POINTS+2,row);
                }
                else
                {
                    pic::logmsg() << "warning: no data for key " << k << " corner " << c;
                }
            }
        }

        pic::logmsg() << "loading calibration done";
    }
    catch(pic::error &e)
    {
        pic::logmsg() << "error reading calibration data: " << e.what();
    }
}

bool micro::active_t::get_calibration(unsigned key, unsigned corner, unsigned short *min, unsigned short *max, unsigned short *table)
{
    unsigned short data[BCTMICRO_CALTABLE_SIZE];

    _impl->control_in(0x80|TYPE_VENDOR,BCTMICRO_USBCOMMAND_CALREAD,corner,key,data,2*BCTMICRO_CALTABLE_SIZE);

    *min = pic_ntohs(data[0]);
    *max = pic_ntohs(data[1]);
    for(unsigned i=0; i<BCTMICRO_CALTABLE_POINTS; ++i)
    {
        table[i] = pic_ntohs(data[i+2]);
    }

    if((data[0]&0x8080) == 0x8080)
        return false;

    return true;
}
    
pic::usbdevice_t *micro::active_t::device()
{
    return _impl;
}   


void micro::active_t::set_raw(bool r)
{
    _impl->raw_=r;
}
