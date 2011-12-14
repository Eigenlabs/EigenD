
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

namespace
{
    static inline void int2c(int r, unsigned char *o)
    {
        long k = r;

        if(r<0) 
        {
            k = (int)0x10000+r;
        }

        unsigned long l = (unsigned long)k;

        o[0] = ((k>>8)&0xff);
        o[1] = (k&0xff);
    }
};

struct piw::statusbuffer_t::impl_t: piw::event_data_source_real_t, piw::thing_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(const piw::change_nb_t &s, unsigned ch, const piw::cookie_t &c): piw::event_data_source_real_t(piw::pathnull(0)), switch_(s), blinking_(false), override_(false), channel_(ch), blink_time_(DEFAULT_BLINK_TIME), blink_size_(0), autosend_(true)
    {
        piw::tsd_thing(this);
        root_.connect(c);
        root_.connect_wire(&wire_, source());
        buffer_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_TINY);
        piw::tsd_fastcall(init__,this,0);
    }

    ~impl_t()
    {
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

    void send_statusbuffer_values()
    {
        unsigned char *dp;
        unsigned long long t = piw::tsd_time();
        unsigned s = statusbuffer_.size()*5;

        piw::data_nb_t result = makeblob_nb(t,s,&dp);

        pic::lckmap_t<std::pair<int,int>,unsigned char>::nbtype::iterator i;

        for(i=statusbuffer_.begin(); i!=statusbuffer_.end(); i++)
        {
            int2c(i->first.first,dp+0);
            int2c(i->first.second,dp+2);
            dp[4] = i->second;
            dp += 5;
        }

        buffer_.add_value(1,result);
    }

    void send_blink_values()
    {
        unsigned char *dp;
        unsigned long long t = piw::tsd_time();
        unsigned s = 5*blink_size_;

        piw::data_nb_t result = makeblob_nb(t,s,&dp);

        for(unsigned n=1;n<=blink_size_;n++)
        {
            int2c(0,dp+0);
            int2c(n,dp+2);
            dp[4] = BCTSTATUS_BLINK;
            dp += 5;
        }

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

    static int set_status__(void *self_, void *row_, void *col_, void *status_)
    {
        impl_t *self = (impl_t *)self_;
        int row = *(int *)row_;
        int col = *(int *)col_;
        unsigned char status = *(unsigned char *)status_;

        pic::lckmap_t<std::pair<int,int>,unsigned char>::nbtype::iterator i;

        i = self->statusbuffer_.find(std::make_pair(row,col));

        if(i==self->statusbuffer_.end())
        {
            if(status)
            {
                self->statusbuffer_.insert(std::make_pair(std::make_pair(row,col),status));

                if(self->autosend_)
                {
                    self->send_statusbuffer_values();
                }
            }

            return 1;
        }

        if(status==0)
        {
            self->statusbuffer_.erase(i);

            if(self->autosend_)
            {
                self->send_statusbuffer_values();
            }

            return 1;
        }

        if(i->second != status)
        {
            i->second = status;

            if(self->autosend_)
            {
                self->send_statusbuffer_values();
            }
        }

        return 1;
    }

    static int get_status__(void *self_, void *row_, void *col_)
    {
        impl_t *self = (impl_t *)self_;
        int row = *(int *)row_;
        int col = *(int *)col_;

        pic::lckmap_t<std::pair<int,int>,unsigned char>::nbtype::iterator i;

        i = self->statusbuffer_.find(std::make_pair(row,col));

        if(i==self->statusbuffer_.end())
        {
            return 0;
        }

        return i->second;
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

        self->statusbuffer_.clear();

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

            self->send_blink_values();

            if(!self->override_)
            {
                self->switch_(piw::makelong_nb(self->channel_,piw::tsd_time()));
            }

            self->timer_fast(self->blink_time_);
        }

        return 1;
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

    void set_status(int row, int col, unsigned char status)
    {
        piw::tsd_fastcall4(set_status__,this,&row,&col,&status);
    }

    unsigned char get_status(int row, int col)
    {
        return (unsigned char)piw::tsd_fastcall3(get_status__,this,&row,&col);
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
    pic::lckmap_t<std::pair<int,int>,unsigned char>::nbtype statusbuffer_;
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

void piw::statusbuffer_t::set_status(int row, int col, unsigned char status)
{
    root_->set_status(row,col,status);
}

unsigned char piw::statusbuffer_t::get_status(int row, int col)
{
    return root_->get_status(row,col);
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

