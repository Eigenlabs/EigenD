
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

#include "osc_transport.h"

osc_plg::osc_recipient_t::osc_recipient_t(const char *host, const char *port, unsigned long long t): time_(t)
{
    address_ = lo_address_new(host,port);
}

osc_plg::osc_recipient_t::~osc_recipient_t()
{
    lo_address_free(address_);
}

bool osc_plg::osc_recipient_t::same_as(const char *host, const char *port)
{
    if(strcmp(port,lo_address_get_port(address_)) != 0)
    {
        return false;
    }

    if(strcmp(host,lo_address_get_hostname(address_)) != 0)
    {
        return false;
    }

    return true;
}

osc_plg::osc_thread_t::osc_thread_t(const std::string &agent): stop_(false), counter_(0), agent_(agent)
{
    snapshot_.save();
}

osc_plg::osc_thread_t::~osc_thread_t()
{
    osc_shutdown();
}

void osc_plg::osc_thread_t::osc_startup()
{
    osc_shutdown();
    stop_ = false;
    run();
}

void osc_plg::osc_thread_t::osc_shutdown()
{
    stop_=true;
    wait();
}

int osc_plg::osc_thread_t::slow_register0__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        lo_address sender = lo_message_get_source(msg);
        self->add_slow_recipient(lo_address_get_hostname(sender),lo_address_get_port(sender));
    }
    return 0;
}

int osc_plg::osc_thread_t::slow_register1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        lo_address sender = lo_message_get_source(msg);
        char portstr[16];
        sprintf(portstr,"%d",argv[1]->i);
        self->add_slow_recipient(lo_address_get_hostname(sender),portstr);
    }
    return 0;
}

int osc_plg::osc_thread_t::slow_register2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        char portstr[16];
        sprintf(portstr,"%d",argv[1]->i);
        self->add_slow_recipient(&argv[2]->s,portstr);
    }
    return 0;
}

int osc_plg::osc_thread_t::fast_register0__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        lo_address sender = lo_message_get_source(msg);
        self->add_fast_recipient(lo_address_get_hostname(sender),lo_address_get_port(sender));
    }
    return 0;
}

int osc_plg::osc_thread_t::fast_register1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        lo_address sender = lo_message_get_source(msg);
        char portstr[16];
        sprintf(portstr,"%d",argv[1]->i);
        self->add_fast_recipient(lo_address_get_hostname(sender),portstr);
    }
    return 0;
}

int osc_plg::osc_thread_t::fast_register2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    osc_thread_t *self = (osc_thread_t *)user_data;

    if(!strcmp(&argv[0]->s,self->agent_.c_str()))
    {
        char portstr[16];
        sprintf(portstr,"%d",argv[1]->i);
        self->add_fast_recipient(&argv[2]->s,portstr);
    }
    return 0;
}

void osc_plg::osc_thread_t::thread_init()
{
    snapshot_.install();
    receiver_ = lo_server_new("9999",0);
    lo_server_add_method(receiver_,"/register","s",slow_register0__,this);
    lo_server_add_method(receiver_,"/register","si",slow_register1__,this);
    lo_server_add_method(receiver_,"/register","sis",slow_register2__,this);
    lo_server_add_method(receiver_,"/register-fast","s",fast_register0__,this);
    lo_server_add_method(receiver_,"/register-fast","si",fast_register1__,this);
    lo_server_add_method(receiver_,"/register-fast","sis",fast_register2__,this);
}

void osc_plg::osc_thread_t::thread_term()
{
    for(unsigned i=0;i<slow_recipients_.size();i++)
    {
        if(slow_recipients_[i])
        {
            delete slow_recipients_[i];
            slow_recipients_[i]=0;
        }
    }

    for(unsigned i=0;i<fast_recipients_.size();i++)
    {
        if(fast_recipients_[i])
        {
            delete fast_recipients_[i];
            fast_recipients_[i]=0;
        }
    }

    lo_server_del_method(receiver_,"/register","s",slow_register0__,this);
    lo_server_del_method(receiver_,"/register","si",slow_register1__,this);
    lo_server_del_method(receiver_,"/register","sis",slow_register2__,this);
    lo_server_del_method(receiver_,"/register-fast","s",fast_register0__,this);
    lo_server_del_method(receiver_,"/register-fast","si",fast_register1__,this);
    lo_server_del_method(receiver_,"/register-fast","sis",fast_register2__,this);
    lo_server_free(receiver_);
}

void osc_plg::osc_thread_t::thread_main()
{
    unsigned counter = 0;

    while (!stop_)
    {
        lo_server_recv_noblock(receiver_, 10);

        if((counter++%100)==0)
        {
            prune();
        }
    }
}

void osc_plg::osc_thread_t::prune()
{
    unsigned long long t = piw::tsd_time()-60000000;
    
    for(unsigned i=0;i<slow_recipients_.size();i++)
    {
        if(slow_recipients_[i] && slow_recipients_[i]->time_ < t)
        {
            pic::logmsg() << "delete recipient " << lo_address_get_hostname(slow_recipients_[i]->address_) << ":" << lo_address_get_port(slow_recipients_[i]->address_);
            delete slow_recipients_[i];
            slow_recipients_[i]=0;
        }
    }

    for(unsigned i=0;i<slow_recipients_.size();i++)
    {
        if(slow_recipients_[i] && slow_recipients_[i]->time_ < t)
        {
            pic::logmsg() << "delete recipient " << lo_address_get_hostname(slow_recipients_[i]->address_) << ":" << lo_address_get_port(slow_recipients_[i]->address_);
            delete slow_recipients_[i];
            slow_recipients_[i]=0;
        }
    }
}

void osc_plg::osc_thread_t::add_slow_recipient(const char *host, const char *port)
{
    int hole = -1;
    unsigned long long t = piw::tsd_time();

    for(unsigned i=0;i<slow_recipients_.size();i++)
    {
        if(!slow_recipients_[i])
        {
            hole = i;
            continue;
        }

        if(slow_recipients_[i]->same_as(host,port))
        {
            slow_recipients_[i]->time_=t;
            return;
        }
    }

    pic::logmsg() << "add recipient " << host << ":" << port;

    osc_recipient_t *b = new osc_recipient_t(host,port,t);
    
    if(hole>=0)
    {
        slow_recipients_[hole] = b;
    }
    else
    {
        slow_recipients_.push_back(b);
    }

}

void osc_plg::osc_thread_t::add_fast_recipient(const char *host, const char *port)
{
    int hole = -1;
    unsigned long long t = piw::tsd_time();

    for(unsigned i=0;i<fast_recipients_.size();i++)
    {
        if(!fast_recipients_[i])
        {
            hole = i;
            continue;
        }

        if(fast_recipients_[i]->same_as(host,port))
        {
            fast_recipients_[i]->time_=t;
            return;
        }
    }

    pic::logmsg() << "add recipient " << host << ":" << port;

    osc_recipient_t *b = new osc_recipient_t(host,port,t);
    
    if(hole>=0)
    {
        fast_recipients_[hole] = b;
    }
    else
    {
        fast_recipients_.push_back(b);
    }

}

bool osc_plg::osc_thread_t::build_channel_url(char *buf, unsigned len, const std::string &port, unsigned index)
{
    char portstr[32];
    sprintf(portstr,"%d",index);
    unsigned pl = 1+agent_.length()+1+port.length()+1+strlen(portstr);

    if(pl>=len)
    {
        return false;
    }

    sprintf(buf,"/%s/%s/%s",agent_.c_str(),port.c_str(),portstr);
    return true;
}

void osc_plg::osc_thread_t::osc_send_fast(const char *name,lo_message m)
{
    // send value to all recipients
    for(unsigned i=0;i<fast_recipients_.size();i++)
    {
        if(!fast_recipients_[i])
        {
            continue;
        }

        lo_send_message_from(fast_recipients_[i]->address_,receiver_,name,m);
    }
}

