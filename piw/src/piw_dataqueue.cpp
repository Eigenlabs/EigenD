
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

#include <piw/piw_dataqueue.h>
#include <piw/piw_tsd.h>

bool piw::dataqueue_t::read(piw::data_nb_t &d,unsigned long long *i,unsigned long long t) const
{
    int rc;
    bct_data_t d2;

    if(!queue_)
    {
        return false;
    }

    rc=bct_dataqueue_read(queue_,&d2,i,t);
    if(rc<0)
    {
        pic::logmsg() << "read failed at " << i; dump(false);
        return false;
    }

    if(rc==0)
    {
        return false;
    }

    d=data_nb_t::from_given(d2);
    return true;
}

bool piw::dataqueue_t::latest(piw::data_nb_t &d, unsigned long long *i, unsigned long long t) const
{
    int rc;
    bct_data_t d2;

    if(!queue_)
    {
        return false;
    }

    rc=bct_dataqueue_latest(queue_,&d2,i,t);
    if(rc<0)
    {
        pic::logmsg() << "read failed";
        return false;
    }

    if(rc==0)
    {
        return false;
    }

    d=data_nb_t::from_given(d2);
    return true;
}

bool piw::dataqueue_t::earliest(piw::data_nb_t &d, unsigned long long *i, unsigned long long t) const
{
    int rc;
    bct_data_t d2;

    if(!queue_)
    {
        return false;
    }

    rc=bct_dataqueue_earliest(queue_,&d2,i,t);
    if(rc<0)
    {
        pic::logmsg() << "read failed";
        return false;
    }

    if(rc==0)
    {
        return false;
    }

    d=data_nb_t::from_given(d2);
    return true;
}

void piw::dataqueue_t::write_fast(const piw::data_nb_t &d)
{
    piw::tsd_protect_t p;
    if(bct_dataqueue_write(queue_,d.lend())<0)
    {
        dump(false);
        pic::msg() << "write failed at time " << d.time() << pic::hurl;
    }
}

int piw::dataqueue_t::write_restamp__(void *q_, void *d_)
{
    piw::dataqueue_t *q = (piw::dataqueue_t *)q_;
    bct_data_t d = (const bct_data_t )d_;
    bct_data_t r = makecopy(PIC_ALLOC_NB, tsd_time(), d);
    piw::tsd_protect_t p;
    int result = bct_dataqueue_write(q->queue_,r);
    return result;
}

void piw::dataqueue_t::write_slow(const piw::data_base_t &d)
{
    PIC_ASSERT(!d.is_null());
    if(piw::tsd_fastcall(write_restamp__,this,d.lend())<0)
    {
        dump(false);
        pic::msg() << "write failed at time " << d.time() << pic::hurl;
    }
}

void piw::dataqueue_t::send_fast__(const piw::data_nb_t &d)
{
    write_fast(d);
}

void piw::dataqueue_t::send_slow__(const piw::data_t &d)
{
    write_slow(d);
}

piw::dataqueue_t::dataqueue_t(bct_dataqueue_t q): queue_(q)
{
}

static int clear__(void *self_, void *arg_)
{
    piw::dataqueue_t *self = (piw::dataqueue_t *)self_;

    self->clear();

    return 1;
}

piw::dataqueue_t::~dataqueue_t()
{
    piw::tsd_fastcall(clear__,this,0);
}

piw::data_nb_t piw::dataqueue_t::current() const
{
    return queue_ ? piw::data_nb_t::from_given(bct_dataqueue_current(queue_)) : data_nb_t();
}

static int trigger__(void *q_, void *)
{
    piw::dataqueue_t *q = (piw::dataqueue_t *)q_;
    unsigned long long t = piw::tsd_time();
    q->write_fast(piw::makebool_nb(true,t));
    q->write_fast(piw::makebool_nb(false,t+1));
    return 0;
}


void piw::dataqueue_t::trigger_slow()
{
    piw::tsd_fastcall(trigger__,this,0);
}
