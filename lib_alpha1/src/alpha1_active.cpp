
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
#include <string.h>

#include <picross/pic_usb.h>
#include <lib_alpha1/alpha1_active.h>
#include <lib_alpha1/alpha1_usb.h>
#include <picross/pic_log.h>
#include <picross/pic_config.h>

#ifndef PI_BIGENDIAN
#define MY_NTOHS(X) (X)
#else
#define MY_NTOHS(X) ((((X)&0xff)<<8)|(((X)>>8)&0xff))
#endif

struct alpha1::active_t::impl_t: pic::usbdevice_t::in_pipe_t, pic::usbdevice_t::power_t, pic::usbdevice_t, virtual public pic::lckobject_t
{
    impl_t(const char *, alpha1::active_t::delegate_t *);
    ~impl_t();

    int decode_keydown(const unsigned short *frame, unsigned length, unsigned long long ts);
    int decode_raw(const unsigned short *frame, unsigned length, unsigned long long ts);
    int decode_processed(const unsigned short *frame, unsigned length, unsigned long long ts);

    void in_pipe_data(const unsigned char *frame, unsigned length, unsigned long long hf, unsigned long long ht,unsigned long long pt);
    void pipe_error(unsigned long long fnum, int err);
    void pipe_died();
    void pipe_started();
    void pipe_stopped();

    void on_suspending();
    void on_waking();

    void start();
    void stop();

    alpha1::active_t::delegate_t *handler_;
    char name_[17];
};

alpha1::active_t::impl_t::impl_t(const char *name, alpha1::active_t::delegate_t *del): usbdevice_t::in_pipe_t(BCTALPHA1_USBENDPOINT_SENSOR_NAME,BCTALPHA1_USBENDPOINT_SENSOR_SIZE), usbdevice_t(name,BCTALPHA1_INTERFACE), handler_(del)
{
    set_power_delegate(this);
    //device_.control_in(BCTALPHA1_USBCOMMAND_GETNAME_REQTYPE,BCTALPHA1_USBCOMMAND_GETNAME_REQ,0,0,name_,16);
    //name_[16]=0;
    strcpy(name_,"alpha");
    add_inpipe(this);
}

alpha1::active_t::impl_t::~impl_t()
{
    stop();
}

alpha1::active_t::active_t(const char *name, alpha1::active_t::delegate_t *handler)
{
    _impl = new impl_t(name,handler);
}

alpha1::active_t::~active_t()
{
    delete _impl;
}

void alpha1::active_t::impl_t::start()
{
    start_pipes();
}

void alpha1::active_t::start()
{
    _impl->start();
}

void alpha1::active_t::impl_t::stop()
{
    stop_pipes();
}

void alpha1::active_t::stop()
{
    _impl->stop();
}

bool alpha1::active_t::poll(unsigned long long t)
{
    return _impl->poll_pipe(t);
}

void alpha1::active_t::set_led(unsigned key, unsigned colour)
{
    try
    {
        _impl->control(BCTALPHA1_USBCOMMAND_SETLED_REQTYPE,BCTALPHA1_USBCOMMAND_SETLED_REQ,colour,key);
    }
    CATCHLOG()
}

void alpha1::active_t::set_raw(bool raw)
{
    if(raw)
        _impl->control(BCTALPHA1_USBCOMMAND_SETRAW_REQTYPE,BCTALPHA1_USBCOMMAND_SETRAW_REQ,0,0);
    else
        _impl->control(BCTALPHA1_USBCOMMAND_SETCOOKED_REQTYPE,BCTALPHA1_USBCOMMAND_SETCOOKED_REQ,0,0);
}

const char *alpha1::active_t::get_name()
{
    return _impl->name_;
}

static void __write16(unsigned char *raw, unsigned i, unsigned short val)
{
    raw[i*2] = (val>>8)&0x00ff;
    raw[i*2+1] = (val>>0)&0x00ff;
}

void alpha1::active_t::set_calibration(unsigned key, unsigned corner, unsigned short min, unsigned short max, const unsigned short *table)
{
    unsigned char raw[2*BCTALPHA1_CALTABLE_SIZE];

    __write16(raw,BCTALPHA1_CALTABLE_MIN,min);
    __write16(raw,BCTALPHA1_CALTABLE_MAX,max);

    for(unsigned i=0;i<BCTALPHA1_CALTABLE_POINTS;i++)
    {
        __write16(raw,BCTALPHA1_CALTABLE_DATA+i,table[i]);
    }

    _impl->control_out(BCTALPHA1_USBCOMMAND_CALDATA_REQTYPE,BCTALPHA1_USBCOMMAND_CALDATA_REQ,corner,key,raw,64);
}

void alpha1::active_t::write_calibration()
{
    _impl->control(BCTALPHA1_USBCOMMAND_CALWRITE_REQTYPE,BCTALPHA1_USBCOMMAND_CALWRITE_REQ,0,0);
}

void alpha1::active_t::clear_calibration()
{
    _impl->control(BCTALPHA1_USBCOMMAND_CALCLEAR_REQTYPE,BCTALPHA1_USBCOMMAND_CALCLEAR_REQ,0,0);
}

unsigned alpha1::active_t::get_temperature()
{
    unsigned char msg[16];
    _impl->control_in(0x80|BCTALPHA1_USBCOMMAND_TEMP_REQTYPE,BCTALPHA1_USBCOMMAND_TEMP_REQ,0,0,msg,16);
    return msg[0];
}

/*
static unsigned long long adjust_time(unsigned long long hf, unsigned long long ht, unsigned short thi, unsigned short tlo)
{
    unsigned short offset = (hf-thi)&0x7ff;
    return tlo + (ht-1000*offset);
}
*/

int alpha1::active_t::impl_t::decode_keydown(const unsigned short *payload, unsigned length, unsigned long long ts)
{
    if(length<BCTALPHA1_MSGSIZE_KEYDOWN)
    {
        return -1;
    }

#ifdef PI_BIGENDIAN
    unsigned short bitmap[BCTALPHA1_PAYLOAD_KEYDOWN];

    for(unsigned i=0;i<11;i++)
    {
        bitmap[i] = MY_NTOHS(payload[i]);
    }

    handler_->kbd_keydown(ts, bitmap);
#else
    handler_->kbd_keydown(ts, payload);
#endif
    return BCTALPHA1_MSGSIZE_KEYDOWN;
}

int alpha1::active_t::impl_t::decode_raw(const unsigned short *payload, unsigned length, unsigned long long ts)
{
    if(length<BCTALPHA1_MSGSIZE_RAW)
    {
        return -1;
    }

    unsigned short key = MY_NTOHS(payload[BCTALPHA1_MSG_RAW_KEY]);
    unsigned short c0 = MY_NTOHS(payload[BCTALPHA1_MSG_RAW_I0]);
    unsigned short c1 = MY_NTOHS(payload[BCTALPHA1_MSG_RAW_I1]);
    unsigned short c2 = MY_NTOHS(payload[BCTALPHA1_MSG_RAW_I2]);
    unsigned short c3 = MY_NTOHS(payload[BCTALPHA1_MSG_RAW_I3]);

    handler_->kbd_raw(ts, key, c0, c1, c2, c3);

    return BCTALPHA1_MSGSIZE_RAW;
}

int alpha1::active_t::impl_t::decode_processed(const unsigned short *payload, unsigned length, unsigned long long ts)
{
    if(length<BCTALPHA1_MSGSIZE_PROCESSED)
    {
        return -1;
    }

    unsigned short key = MY_NTOHS(payload[BCTALPHA1_MSG_PROCESSED_KEY]);
    unsigned short pressure = MY_NTOHS(payload[BCTALPHA1_MSG_PROCESSED_P]);
    unsigned short roll = MY_NTOHS(payload[BCTALPHA1_MSG_PROCESSED_R]);
    unsigned short yaw = MY_NTOHS(payload[BCTALPHA1_MSG_PROCESSED_Y]);

    handler_->kbd_key(ts, key, pressure, roll, yaw);
    return BCTALPHA1_MSGSIZE_PROCESSED;
}

//#define DEBUG_FRAME 1

void alpha1::active_t::impl_t::in_pipe_data(const unsigned char *frame, unsigned length, unsigned long long hf, unsigned long long ht,unsigned long long pt)
{
    unsigned l=length/2;
    unsigned short o;

#ifdef DEBUG_FRAME

    for(o=0; o<l; ++o)
    {
        if((o%32)==0)
            printf("\n");
        printf("%04x ",MY_NTOHS(p[o]));
    }
    printf("\n-----------\n");

#else

    const unsigned short *p = (const unsigned short *)frame;

    while(l>BCTALPHA1_HEADER_SIZE)
    {
        pik_msg_t *m = (pik_msg_t *)p;
        switch(m->type)
        {
            case 0: case BCTALPHA1_MSGTYPE_NULL:          return;
            case BCTALPHA1_MSGTYPE_KEYDOWN:       o=decode_keydown(m->payload,l,ht); break;
            case BCTALPHA1_MSGTYPE_RAW:           o=decode_raw(m->payload,l,ht); break;
            case BCTALPHA1_MSGTYPE_PROCESSED:     o=decode_processed(m->payload,l,ht); break;

            default: pic::logmsg() << "invalid usb message type " << (unsigned)m->type; return;
        }

        p+=o;
        l-=o;
        ht+=10;
    }

#endif
}

void alpha1::active_t::impl_t::pipe_died()
{
    pipe_stopped();
    handler_->kbd_dead();
}

void alpha1::active_t::impl_t::pipe_stopped()
{
    try
    {
        control_out(BCTALPHA1_USBCOMMAND_STOP_REQTYPE,BCTALPHA1_USBCOMMAND_STOP_REQ,0,0,0,0);
    }
    catch(...)
    {
        pic::logmsg() << "device shutdown command failed";
    }

}

void alpha1::active_t::impl_t::pipe_started()
{
    pic::logmsg() << "starting up keyboard";
    control_out(BCTALPHA1_USBCOMMAND_START_REQTYPE,BCTALPHA1_USBCOMMAND_START_REQ,0,0,0,0);
    pic::logmsg() << "started up keyboard";
}

void alpha1::active_t::impl_t::pipe_error(unsigned long long fnum, int err)
{
    pic::msg() << "usb error in frame " << fnum << " err=" << std::hex << err << pic::log;
}

void alpha1::active_t::impl_t::on_waking()
{
    pic::logmsg() << "starting up keyboard";

    try
    {
        control_out(BCTALPHA1_USBCOMMAND_START_REQTYPE,BCTALPHA1_USBCOMMAND_START_REQ,0,0,0,0);
    }
    catch(...)
    {
        pic::logmsg() << "device startup failed";
    }

    pic::logmsg() << "started up keyboard";
}

void alpha1::active_t::impl_t::on_suspending()
{
    pic::logmsg() << "shutting down kbd";

    try
    {
        control_out(BCTALPHA1_USBCOMMAND_STOP_REQTYPE,BCTALPHA1_USBCOMMAND_STOP_REQ,0,0,0,0);
    }
    catch(...)
    {
        pic::logmsg() << "device shutdown failed";
    }

    pic::logmsg() << "shut down kbd";
}

pic::usbdevice_t *alpha1::active_t::device()
{
    return _impl;
}
