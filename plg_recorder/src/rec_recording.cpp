
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


#include "rec_recording.h"
#include <pirecorder_exports.h>
#include <piw/piw_address.h>
#include <piembedded/pie_wire.h>
#include <picross/pic_log.h>
#include <picross/pic_resources.h>
#include <pibelcanto/state.h>
#include <math.h>

#include <picross/pic_config.h>
#ifndef PI_WINDOWS
#include <unistd.h>
#else
#include <io.h>
#endif


#include <set>

/*
 *  Record file format:
 *      header: (8)
 *          magic-number    u16     0xbeca
 *          version         u16     0x0001
 *          signals         u16 
 *          wires           u16
 *      tags: (4+data-len)
 *          tag-type        u8      (0 for end of tags)
 *          data-type       u8
 *          data-len        u16
 *          data            u8[data-len]
 *      samples: (16+data-len)
 *          timestamp       u32     (microseconds)
 *          beat-stamp      f32
 *          bar-stamp       f32
 *          signal          u16     (0 for event)
 *          data-len        u16
 *          data            u8[data-len]
 */

static bool __write(FILE *fp, const recorder::dataref_t &r)
{
    unsigned char buffer[BCTLINK_MAXPAYLOAD];
    memset(buffer,0,BCTLINK_MAXPAYLOAD);

    pie_setu16(buffer+0,2,0xbeca);
    pie_setu16(buffer+2,2,0x0001);

    if(!r.isvalid())
    {
        if(fwrite(buffer,9,1,fp)!=1) return false;
        return true;
    }

    pie_setu16(buffer+4,2,r->signals_);
    pie_setu16(buffer+6,2,r->wires_);

    if(fwrite(buffer,8,1,fp)!=1) return false;

    recorder::taglist_t::const_iterator ti;

    for(ti=r->tags_.begin(); ti!=r->tags_.end(); ti++)
    {
        unsigned wl = ti->second.wire_length();

        buffer[0] = ti->first;
        buffer[1] = ti->second.type();
        pie_setu16(buffer+2,2,wl);
        memcpy(&buffer[4],ti->second.wire_data(),wl);

        if(fwrite(buffer,4+wl,1,fp)!=1) return false;
    }

    buffer[0] = 0;
    if(fwrite(buffer,1,1,fp)!=1) return false;

    recorder::samplelist_t::const_iterator si;
    recorder::eventlist_t::const_iterator ei;

    for(ei=r->events_.begin(); ei!=r->events_.end(); ei++)
    {
        recorder::event_t *e = ei->ptr();

        unsigned wl = e->value.wire_length();

        pie_setu32(buffer+ 0,4,e->time);
        pie_setf32(buffer+ 4,4,e->beat);
        pie_setf32(buffer+ 8,4,e->beat);
        pie_setu16(buffer+12,2,0);
        pie_setu16(buffer+14,2,wl);
        memcpy(buffer+16,e->value.wire_data(),wl);

        if(fwrite(buffer,16+wl,1,fp)!=1)
        {
            return false;
        }

        for(si=e->samples_.begin(); si!=e->samples_.end(); si++)
        {
            unsigned wl = si->value.wire_length();

            pie_setu32(buffer+ 0,4,si->time);
            pie_setf32(buffer+ 4,4,si->beat);
            pie_setf32(buffer+ 8,4,si->beat);
            pie_setu16(buffer+12,2,si->signal);
            pie_setu16(buffer+14,2,wl);
            memcpy(buffer+16,si->value.wire_data(),wl);

            if(fwrite(buffer,16+wl,1,fp)!=1)
            {
                return false;
            }
        }
    }

    return true;
}

void recorder::recording_t::write(const char *filename) const
{
    FILE *fp;

    if(!(fp=pic::fopen(filename,"wb")))
    {
        pic::msg() << "can\'t open " << filename << pic::hurl;
    }

    if(!__write(fp,data_))
    {
        fclose(fp);
        pic::remove(filename);
        pic::msg() << "can\'t write " << filename << pic::hurl;
    }

    fclose(fp);
}

static recorder::dataref_t __read(FILE *fp,bool justmeta)
{
    unsigned char buffer[BCTLIMIT_DATA];
    unsigned short s,w;

    if(fread(buffer,8,1,fp)!=1) return recorder::dataref_t();
    if(pie_getu16(buffer+0,2,&s)<0 || s!=0xbeca) return recorder::dataref_t();
    if(pie_getu16(buffer+2,2,&s)<0 || s!=0x0001) return recorder::dataref_t();
    if(pie_getu16(buffer+4,2,&s)<0) return recorder::dataref_t();
    if(pie_getu16(buffer+6,2,&w)<0) return recorder::dataref_t();

    recorder::dataref_t data = pic::ref(new recorder::recording_data_t(s,w));

    while(true)
    {
        unsigned char x;
        unsigned short l;

        if(fread(buffer,1,1,fp) != 1) return recorder::dataref_t();

        if((x=buffer[0])==0)
        {
            break;
        }

        if(fread(buffer,3,1,fp) != 1) return recorder::dataref_t();
        if(pie_getu16(buffer+1,2,&l)<0 || l>BCTLIMIT_DATA) return recorder::dataref_t();
        if(fread(buffer,l,1,fp) != 1) return recorder::dataref_t();

        data->tags_.insert(std::make_pair(x,piw::makewire_nb(l,buffer)));
    }

    if(justmeta)
        return data;

    pic::ref_t<recorder::event_t> cur_event;

    while(true)
    {
        uint32_t t;
        float b1,b2;
        uint16_t l;

        if(fread(buffer,16,1,fp) != 1)
        {
            if(feof(fp))
            {
                break;
            }

            return recorder::dataref_t();
        }

        if(pie_getu32(buffer+ 0,4,&t)<0) return recorder::dataref_t();
        if(pie_getf32(buffer+ 4,4,&b1)<0) return recorder::dataref_t();
        if(pie_getf32(buffer+ 8,4,&b2)<0) return recorder::dataref_t();
        if(pie_getu16(buffer+12,2,&s)<0) return recorder::dataref_t();
        if(pie_getu16(buffer+14,2,&l)<0) return recorder::dataref_t();

        if(l>0 && fread(buffer,l,1,fp) != 1) return recorder::dataref_t();

        piw::data_nb_t v = piw::makewire_nb(l,buffer);

        if(s==0)
        {
            cur_event = pic::ref(new recorder::event_t(t,b1,v));
            cur_event->iscomplete_=true;
            data->events_.push_back(cur_event);
        }
        else
        {
            if(!cur_event.isvalid())
            {
                return recorder::dataref_t();
            }

            cur_event->samples_.push_back(recorder::sample_t(t,b1,s,v));
            cur_event->max_beat = b1;
            cur_event->max_time = t;
        }
    }

    return data;
}

recorder::recording_t recorder::read(const char *filename)
{
    FILE *fp;

    if(!(fp=pic::fopen(filename,"rb")))
    {
        pic::msg() << "can\'t open " << filename << pic::hurl;
    }

    recorder::dataref_t d = __read(fp,false);
    fclose(fp);

    if(!d.isvalid())
    {
        pic::msg() << "can\'t read " << filename << pic::hurl;
    }

    return recorder::recording_t(d);
}

recorder::recording_t recorder::read_meta(const char *filename)
{
    FILE *fp;

    if(!(fp=pic::fopen(filename,"rb")))
    {
        pic::msg() << "can\'t open " << filename << pic::hurl;
    }

    recorder::dataref_t d = __read(fp,true);
    fclose(fp);

    if(!d.isvalid())
    {
        pic::msg() << "can\'t read " << filename << pic::hurl;
    }

    return recorder::recording_t(d);
}

recorder::recording_data_t::recording_data_t(unsigned signals, unsigned wires) : signals_(signals), wires_(wires)
{
}

recorder::recordmaker_t::recordmaker_t()
{
}

void recorder::recordmaker_t::new_recording(unsigned signals, unsigned wires)
{
    data_ = pic::ref(new recording_data_t(signals,wires));
}

void recorder::recordmaker_t::set_tag(unsigned char tag, const piw::data_nb_t &d)
{
    data_->tags_.insert(std::make_pair(tag,d));
}

static void __add_value(pic::ref_t<recorder::event_t> &event, unsigned long long time, float beat, unsigned signal, const piw::data_nb_t &value)
{
    recorder::samplelist_t::reverse_iterator i, b, e;

    b=event->samples_.rbegin();
    e=event->samples_.rend();

    for(i=b; i!=e; ++i)
    {
        if(i->time < time)
        {
            break;
        }
    }

    if(i==b)
    {
        event->samples_.push_back(recorder::sample_t(time,beat,signal,value));
    }
    else
    {
        event->samples_.insert(i.base(),recorder::sample_t(time,beat,signal,value));
    }
}

static void __end_event(pic::ref_t<recorder::event_t> &event, unsigned long long time, float beat)
{
    if(event->iscomplete_)
    {
        return;
    }

    recorder::samplelist_t::reverse_iterator i, b, e;

    __add_value(event,time,beat,256,piw::makenull_nb());

    b=event->samples_.rbegin();
    e=event->samples_.rend();

    if(b==e)
    {
        event->max_beat=0;
        event->max_time=0;
    }
    else
    {
        event->max_beat=b->beat;
        event->max_time=b->time;
    }

    event->iscomplete_=true;
}

void recorder::writeevent_t::end_event(unsigned long long time, float beat)
{
    if(event_.isvalid())
    {
        __end_event(event_,time,beat);
        event_.clear();
    }
}

void recorder::writeevent_t::add_value(unsigned long long time, float beat, unsigned signal, const piw::data_nb_t &value)
{
    __add_value(event_,time,beat,signal,value);
}

recorder::writeevent_t recorder::recordmaker_t::new_event(unsigned long long time, float beat, const piw::data_nb_t &value)
{
    eventlist_t::reverse_iterator i, b, e;
    b=data_->events_.rbegin();
    e=data_->events_.rend();

    for(i=b; i!=e; ++i)
    {
        if((*i)->time < time)
        {
            break;
        }
    }

    pic::ref_t<event_t> event = pic::ref(new event_t(time,beat,value));

    if(i==b)
    {
        data_->events_.push_back(event);
    }
    else
    {
        data_->events_.insert(i.base(),event);
    }

    return writeevent_t(event);
}

recorder::recording_t recorder::recordmaker_t::get_recording()
{
    return recording_t(data_);
}

recorder::recording_t::recording_t()
{
}

recorder::recording_t::recording_t(const recorder::recording_t &other)
{
    *this = other;
}

recorder::recording_t &recorder::recording_t::operator=(const recorder::recording_t &other)
{
    data_ = other.data_;
    reset();

    return *this;
}

recorder::recording_t::recording_t(const dataref_t &d) : data_(d)
{
    reset();
}

unsigned recorder::recording_t::signals() const
{
    return data_.isvalid() ? data_->signals_ : 0;
}

unsigned recorder::recording_t::wires() const
{
    return data_.isvalid() ? data_->wires_ : 0;
}


piw::data_t recorder::recording_t::get_tag(unsigned char tag) const
{
    taglist_t::const_iterator i = data_->tags_.find(tag);
    if(i != data_->tags_.end())
    {
        return i->second.make_normal();
    }
    return piw::data_t();
}

recorder::readevent_t recorder::recording_t::cur_event() const
{
    if(!event_.isvalid())
    {
        return readevent_t();
    }

    return readevent_t(event_);
}

bool recorder::recording_t::isvalid() const
{
    return data_.isvalid() && current_event_ != data_->events_.end();
}

void recorder::recording_t::reset()
{
    if(!data_.isvalid())
    {
        return;
    }

    current_event_ = data_->events_.begin();

    if(current_event_==data_->events_.end())
    {
        return;
    }

    event_ = *current_event_;
}

void recorder::recording_t::next()
{
    if(!isvalid())
    {
        return;
    }

    ++current_event_;

    if(current_event_==data_->events_.end())
    {
        return;
    }

    event_ = *current_event_;

    if(!event_->iscomplete_)
    {
        __end_event(event_,data_->current_time_,data_->current_beat_);
    }
}

bool recorder::readevent_t::isvalid() const
{
    return event_.isvalid() && current_sample_ != event_->samples_.end();
}

void recorder::readevent_t::clear()
{
    event_.clear();
}

void recorder::readevent_t::reset()
{
    if(!event_.isvalid())
    {
        return;
    }

    current_sample_ = event_->samples_.begin();

    if(current_sample_==event_->samples_.end())
    {
        return;
    }
}

void recorder::readevent_t::next()
{
    if(!isvalid())
    {
        return;
    }

    ++current_sample_;

    if(current_sample_==event_->samples_.end())
    {
        return;
    }
}
