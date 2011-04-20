
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

#include <piw/piw_tsd.h>
#include <piw/piw_status.h>
#include <piw/piw_thing.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_ref.h>

#define DEFAULT_BLINK_TIME 500

struct piw::statusbuffer_t::impl_t: piw::event_data_source_real_t, piw::thing_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(const piw::change_nb_t &s, unsigned ch, const piw::cookie_t &c): piw::event_data_source_real_t(piw::pathnull(0)), switch_(s), blinking_(false), override_(false), channel_(ch), size_(0), blink_time_(DEFAULT_BLINK_TIME), blink_size_(0), statusbuffer_(0), autosend_(true)
    {
        piw::tsd_thing(this);
        root_.connect(c);
        root_.connect_wire(&wire_, source());
        buffer_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        piw::tsd_fastcall(init__,this,0);
    }

    ~impl_t()
    {
        if(statusbuffer_)
        {
            pic::nb_free(statusbuffer_);
        }
        tracked_invalidate();
    }

    static int init__(void *self_, void *arg_)
    {
        impl_t *self = (impl_t *)self_;

        unsigned long long t = piw::tsd_time();
        self->buffer_.add_value(1,makeblob_nb(t,0,0));
        self->source_start(0,piw::pathnull_nb(t),self->buffer_);
        
        return 1;
    }

    void ensure_size(unsigned size)
    {
        if(size_ >= size)
        {
            return;
        }
        
        if(0 == size)
        {
            if(statusbuffer_)
            {
                pic::nb_free(statusbuffer_);
                statusbuffer_ = 0;
            } 
            size_ = 0;
            return;
        }

        unsigned char* new_statusbuffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_NB, size);
        memset(new_statusbuffer_, BCTSTATUS_OFF, size);
        if(statusbuffer_)
        {
            memcpy(new_statusbuffer_, statusbuffer_, std::min(size_, size));
            pic::nb_free(statusbuffer_);
        }

        size_ = size;
        statusbuffer_ = new_statusbuffer_;
        blink_size_ = std::max(size_,blink_size_);
    }

    void send_statusbuffer_values(unsigned size, unsigned char *statusbuffer)
    {
        unsigned char *dp;
        unsigned long long t = piw::tsd_time();
        piw::data_nb_t result = makeblob_nb(t,size,&dp);
        memcpy(dp,statusbuffer,size);

        buffer_.add_value(1,result);
    }

    void send_blink_values(unsigned size)
    {
        unsigned char *dp;
        unsigned long long t = piw::tsd_time();
        piw::data_nb_t result = makeblob_nb(t,size,&dp);
        memset(dp,BCTSTATUS_BLINK,size);

        buffer_.add_value(1,result);
    }

    static int send__(void *self_, void *arg_)
    {
        impl_t *self = (impl_t *)self_;

        self->send_statusbuffer_values();
        
        return 1;
    }

    static int override__(void *self_, void *override_)
    {
        bool override = *(bool *)override_;
        impl_t *self = (impl_t *)self_;

        if(override==self->override_)
        {
            return 1;
        }

        self->override_=override;

        if(!self->blinking_)
        {
            if(override)
            {
                self->switch_(piw::makelong_nb(self->channel_,piw::tsd_time()));
            }
            else
            {
                self->switch_(self->current_.get().restamp(piw::tsd_time()));
            }
        }

        return 1;
    }

    static int set_status__(void *self_, void *index_, void *status_)
    {
        impl_t *self = (impl_t *)self_;
        unsigned index = *(unsigned *)index_;
        unsigned char status = *(unsigned char *)status_;

        if(0 == index)
        {
            return 0;
        }

        self->ensure_size(index);
        if(self->statusbuffer_[index-1] != status)
        {
            self->statusbuffer_[index-1] = status;
            if(self->autosend_)
            {
                self->send_statusbuffer_values();
            }
        }
        
        return 1;
    }

    static int get_status__(void *self_, void *index_)
    {
        impl_t *self = (impl_t *)self_;
        unsigned index = *(unsigned *)index_;

        if(0 == index || index > self->size_)
        {
            return 0;
        }

        return self->statusbuffer_[index-1];
    }

    static int set_blink_time__(void *self_, void *time_)
    {
        float time = *(float *)time_;
        impl_t *self = (impl_t *)self_;

        self->blink_time_=unsigned(time*1000);
        return 1;
    }

    static int set_blink_size__(void *self_, void *size_)
    {
        unsigned size = *(unsigned *)size_;
        impl_t *self = (impl_t *)self_;

        self->blink_size_=size;
        return 1;
    }

    static int autosend__(void *self_, void *autosend_)
    {
        bool autosend = *(bool *)autosend_;
        impl_t *self = (impl_t *)self_;

        self->autosend_=autosend;
        return 1;
    }

    static int clear__(void *self_, void *arg_)
    {
        impl_t *self = (impl_t *)self_;

        for(unsigned i=0; i<self->size_;i++)
        {
            self->statusbuffer_[i]=BCTSTATUS_OFF;
        }

        if(self->autosend_)
        {
            self->send_statusbuffer_values();
        }

        return 1;
    }

    static int enable__(void *r_, void *i_)
    {
        impl_t *self = (impl_t *)r_;
        unsigned input = *(unsigned *)i_;
        self->enabler(piw::makelong_nb(input,piw::tsd_time()));
        return 0;
    }

    static int blink__(void *self_, void *)
    {
        impl_t *self = (impl_t *)self_;

        if(0==unsigned(self->blink_time_))
        {
            self->send_statusbuffer_values();
        }
        else
        {
            pic::logmsg() << "blink..";

            self->blinking_=true;

            self->send_blink_values(self->blink_size_);

            if(!self->override_)
            {
                self->switch_(piw::makelong_nb(self->channel_,piw::tsd_time()));
            }

            self->timer_fast(self->blink_time_);
        }
        return 1;
    }
    
    void send_statusbuffer_values()
    {
        send_statusbuffer_values(size_,statusbuffer_);
    }

    void send()
    {
        piw::tsd_fastcall(send__,this,0);
    }

    void override(bool override)
    {
        piw::tsd_fastcall(override__,this,&override);
    }

    void autosend(bool autosend)
    {
        piw::tsd_fastcall(autosend__,this,&autosend);
    }

    void clear()
    {
        piw::tsd_fastcall(clear__,this,0);
    }

    void set_status(unsigned index, unsigned char status)
    {
        piw::tsd_fastcall3(set_status__,this,&index,&status);
    }

    unsigned char get_status(unsigned index)
    {
        return (unsigned char)piw::tsd_fastcall(get_status__,this,&index);
    }

    void set_blink_time(float time)
    {
        piw::tsd_fastcall(set_blink_time__,this,&time);
    }

    void set_blink_size(unsigned size)
    {
        piw::tsd_fastcall(set_blink_size__,this,&size);
    }

    void enabler(const piw::data_nb_t &d)
    {
        current_.set_nb(d);

        if(!blinking_ && !override_)
        {
            switch_(d);
        }
    }

    void enable(unsigned d)
    {
        piw::tsd_fastcall(enable__,this,&d);
    }

    void blinker(const piw::data_nb_t &d)
    {
        if(d.is_null() || d.as_norm()==0 || blinking_)
        {
            return;
        }
        piw::tsd_fastcall(blink__,this,0);
    }

    void thing_timer_fast()
    {
        if(!blinking_)
        {
            return;
        }

        blinking_=false;
        cancel_timer_fast();

        if(!override_)
        {
            switch_(current_.get().restamp(piw::tsd_time()));
        }

        send_statusbuffer_values();
    }

    piw::root_ctl_t root_;
    piw::wire_ctl_t wire_;
    piw::xevent_data_buffer_t buffer_;
    pic::flipflop_functor_t<piw::change_nb_t> switch_;
    bool blinking_,override_;
    piw::dataholder_nb_t current_;
    unsigned channel_;
    unsigned size_;
    unsigned blink_time_;
    unsigned blink_size_;
    unsigned char *statusbuffer_;
    bool autosend_;
};

piw::change_nb_t piw::statusbuffer_t::enabler()
{
    return piw::change_nb_t::method(root_,&impl_t::enabler);
}

piw::change_nb_t piw::statusbuffer_t::blinker()
{
    return piw::change_nb_t::method(root_,&impl_t::blinker);
}

piw::statusbuffer_t::statusbuffer_t(const piw::change_nb_t &s, unsigned ch, const cookie_t &c): root_(new impl_t(s,ch,c))
{
}

piw::statusbuffer_t::~statusbuffer_t()
{
    delete root_;
}

void piw::statusbuffer_t::send()
{
    root_->send();
}

void piw::statusbuffer_t::override(bool over)
{
    root_->override(over);
}

void piw::statusbuffer_t::autosend(bool autosend)
{
    root_->autosend(autosend);
}

void piw::statusbuffer_t::clear()
{
    root_->clear();
}

void piw::statusbuffer_t::set_status(unsigned index, unsigned char status)
{
    root_->set_status(index,status);
}

unsigned char piw::statusbuffer_t::get_status(unsigned index)
{
    return root_->get_status(index);
}

void piw::statusbuffer_t::set_blink_time(float time)
{
    root_->set_blink_time(time);
}

void piw::statusbuffer_t::set_blink_size(unsigned size)
{
    root_->set_blink_size(size);
}

int piw::statusbuffer_t::gc_traverse(void *v, void *a) const
{
    return root_->switch_.gc_traverse(v,a);
}

int piw::statusbuffer_t::gc_clear()
{
    return root_->switch_.gc_clear();
}

void piw::statusbuffer_t::enable(unsigned input)
{
    root_->enable(input);
}

static const char *__SbHexDigits = "0123456789abcdef";

void piw::statusbuffer_t::dump(std::ostream &o)
{
    unsigned size=root_->size_;
    unsigned char *buf=root_->statusbuffer_;
    while(size>0)
    {
        unsigned char b=*buf;
        o << __SbHexDigits[b/16];
        o << __SbHexDigits[b%16];
        buf++; size--;
    }
}

std::ostream &operator<<(std::ostream &o, const piw::statusbuffer_t &s)
{
    const_cast<piw::statusbuffer_t &>(s).dump(o);
    return o;
}
