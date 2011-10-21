
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

#include "lng_osc.h"
#include <picross/pic_thread.h>
#include <picross/pic_safeq.h>
#include <lib_lo/lo/lo.h>
#include <vector>
#include <cmath>

#define OSC_DEBUG 0
#define OSC_RENDEZVOUS_PORT "55554"

namespace
{
    struct widget_t: piw::fastdata_t
    {
        widget_t(language::oscserver_t::impl_t *impl, const char *name,piw::fastdata_t *f,piw::fastdata_t *f2);
        bool fastdata_receive_data(const piw::data_nb_t &d);
        bool fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q);
        ~widget_t();

        static void sender__(void *a1,void *a2,void *a3,void *a4);
        static int method1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
        static int method2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
        static int method3__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);

        language::oscserver_t::impl_t *impl_;
        const char *name_;
        piw::fastdata_t receiver_;
        lo_method method1_;
        lo_method method2_;
        lo_method method3_;
        piw::dataqueue_t queue_;
        float current_;
        bool is_connected_;
    };

    struct recipient_t
    {
        recipient_t(lo_address a, unsigned long long t);
        ~recipient_t();
        bool same_as(lo_address a);

        lo_address address_;
        unsigned long long time_;
    };

    struct service_broadcast_t 
    {
        service_broadcast_t(language::oscserver_t::impl_t *impl);
        ~service_broadcast_t();

        void receive();
        void shutdown();
        
        static int request_addr__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
        
        language::oscserver_t::impl_t *impl_;
        lo_server broadcast_server_;
        lo_method request_addr_;

    };


};

struct language::oscserver_t::impl_t: pic::thread_t
{
    impl_t(const char *server_port, const char *xmlrpc_server_port);
    ~impl_t();

    void shutdown();
    void thread_main();
    void thread_init();
    void thread_term();
    void prune();
    unsigned add_widget(const char *, piw::fastdata_t *, piw::fastdata_t *);
    void del_widget(unsigned);
    void add_recipient(lo_address);
    void send(const char *name,float v);
    void set_connected(unsigned index, bool is_connected);

    bool stop_;
    std::string server_port_;
    std::string xmlrpc_server_port_;
    piw::tsd_snapshot_t snapshot_;
    pic::safeq_t queue_;
    lo_server receiver_;
    std::vector<widget_t *> widgets_;
    std::vector<recipient_t *> recipients_;
    service_broadcast_t service_broadcast_;
    
    pic::mutex_t update_;
    
};

language::oscserver_t::oscserver_t(const char *server_port, const char *xmlrpc_server_port): impl_(new impl_t(server_port, xmlrpc_server_port))
{
}

language::oscserver_t::~oscserver_t()
{
    delete impl_;
}

unsigned language::oscserver_t::add_widget(const char *name, piw::fastdata_t *data1, piw::fastdata_t *data2)
{
    return impl_->add_widget(name,data1,data2);
}

void language::oscserver_t::del_widget(unsigned index)
{
    impl_->del_widget(index);
}

void language::oscserver_t::send(const char *name,float v)
{
    impl_->send(name,v);
}

void language::oscserver_t::set_connected(unsigned index, bool is_connected)
{
    impl_->set_connected(index, is_connected);
}


unsigned language::oscserver_t::impl_t::add_widget(const char *name, piw::fastdata_t *data1, piw::fastdata_t *data2)
{
    unsigned i;

    for(i=0;i<widgets_.size();i++)
    {
        if(!widgets_[i])
        {
            widgets_[i] = new widget_t(this,name,data1,data2);
            //pic::logmsg() << "oscserver_t::add_widget "<<name << " index="<<i+1 << " data1="<<(void*)data1 << " data2="<<(void*)data2;
            return i+1;
        }
    }

    i=widgets_.size();
    widgets_.push_back(new widget_t(this,name,data1,data2));

    //pic::logmsg() << "oscserver_t::add_widget "<<name << " index="<<i+1 << " data1="<<(void*)data1 << " data2="<<(void*)data2;
    return i+1;
}

void language::oscserver_t::impl_t::del_widget(unsigned index)
{
    //pic::logmsg() << "oscserver_t::del_widget index="<<index;

    if(index>0 && index<=widgets_.size())
    {
        index -= 1;
        if(widgets_[index])
        {
            delete widgets_[index];
            widgets_[index]=0;
            return;
        }
    }
}

void language::oscserver_t::startup()
{
    impl_->run();   
}

void language::oscserver_t::shutdown()
{
    impl_->shutdown();
}

void language::oscserver_t::impl_t::shutdown()
{
    stop_=true;
    wait();
    service_broadcast_.shutdown();
    pic::logmsg() << "oscserver shut down";
}

language::oscserver_t::impl_t::impl_t(const char *server_port, const char *xmlrpc_server_port): 
    stop_(false), server_port_(server_port), xmlrpc_server_port_(xmlrpc_server_port), service_broadcast_(this)
{
    snapshot_.save();
}

language::oscserver_t::impl_t::~impl_t()
{
    shutdown();

    for(unsigned i=0;i<widgets_.size();i++)
    {
        if(widgets_[i])
        {
            delete widgets_[i];
        }
    }
}

void language::oscserver_t::impl_t::thread_init()
{
    snapshot_.install();
    receiver_ = lo_server_new(server_port_.c_str(),0);
}

void language::oscserver_t::impl_t::thread_term()
{
    for(unsigned i=0;i<recipients_.size();i++)
    {
        if(recipients_[i])
        {
            delete recipients_[i];
            recipients_[i]=0;
        }
    }

    lo_server_free(receiver_);
}

void language::oscserver_t::impl_t::thread_main()
{
#ifdef DEBUG_DATA_ATOMICITY
    std::cout << "Started OSC server thread with ID " << pic_current_threadid() << std::endl;
#endif

    unsigned counter = 0;

    while (!stop_)
    {
        service_broadcast_.receive();

        lo_server_recv_noblock(receiver_, 10);
        queue_.run();

        if((counter++%100)==0)
        {
            prune();
        }
    }
}

void language::oscserver_t::impl_t::prune()
{
    unsigned long long t = piw::tsd_time()-60000000;
    
    for(unsigned i=0;i<recipients_.size();i++)
    {
        if(recipients_[i] && recipients_[i]->time_ < t)
        {
            pic::logmsg() << "delete recipient " << lo_address_get_hostname(recipients_[i]->address_)
                          << ":" << lo_address_get_port(recipients_[i]->address_);

            delete recipients_[i];
            recipients_[i]=0;
        }
    }
}

int widget_t::method1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    widget_t *widget = (widget_t *)user_data;
    float f = argv[0]->f;
    float m = fabs(f);
    unsigned long long t = piw::tsd_time();

    piw::data_nb_t d = piw::makefloat_bounded_units_nb(BCTUNIT_GLOBAL,m,-m,0,f,t+1);
#if OSC_DEBUG==1
    pic::logmsg() << "osc receiver " << path << " " << f << "->" << d << ' ' << d.units();
#endif // OSC_DEBUG==1
    
    widget->queue_.write_slow(d);
    widget->impl_->add_recipient(lo_message_get_source(msg));

    if(widget->current_==f)
    {
        widget->impl_->send(widget->name_,widget->current_);
    }
    
    return 0;
}

int widget_t::method2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    widget_t *widget = (widget_t *)user_data;
    language::oscserver_t::impl_t *impl = widget->impl_;
    impl->add_recipient(lo_message_get_source(msg));
    impl->send(widget->name_,widget->current_);

    // send whether connected or not
    std::string name_str(widget->name_);
    name_str+="/connected";
    impl->update_.lock();
    impl->send(name_str.c_str(), widget->is_connected_?1.0:0.0);
    impl->update_.unlock();
    return 0;
}

int widget_t::method3__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    widget_t *widget = (widget_t *)user_data;
    language::oscserver_t::impl_t *impl = widget->impl_;
    impl->add_recipient(lo_message_get_source(msg));
    return 0;
}

widget_t::widget_t(language::oscserver_t::impl_t *impl, const char *name,piw::fastdata_t *f,piw::fastdata_t *f2): 
    impl_(impl), name_(strdup(name)), receiver_(PLG_FASTDATA_SENDER), is_connected_(false)
{
    current_=0;
    piw::tsd_fastdata(this);
    set_upstream(f);
    enable(true,true,false);
    piw::tsd_fastdata(&receiver_);
    f2->set_upstream(&receiver_);
    method1_ = lo_server_add_method(impl_->receiver_,name,"f",method1__,this);
    method2_ = lo_server_add_method(impl_->receiver_,name,"",method2__,this);
    method3_ = lo_server_add_method(impl_->receiver_,"/ping","",method3__,this);
    queue_ = piw::tsd_dataqueue(16);
    receiver_.send_slow(piw::pathnull(piw::tsd_time()),queue_);
}

void widget_t::sender__(void *a1,void *a2,void *a3,void *a4)
{
#if OSC_DEBUG==1
    piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)a2);
    pic::logmsg() << "data =" << d;
#endif // OSC_DEBUG==1
    
    widget_t *widget  = (widget_t *)a1;
    widget->current_ = piw::data_t::from_given((bct_data_t)a2).as_denorm();
    language::oscserver_t::impl_t *impl = widget->impl_;
    impl->send(widget->name_,widget->current_);
#if OSC_DEBUG==1
    pic::logmsg() << "sent queued data " << widget->name_ << " " << widget->current_;
#endif // OSC_DEBUG==1
}

bool widget_t::fastdata_receive_data(const piw::data_nb_t &d)
{
#if OSC_DEBUG==1
    pic::logmsg() << "received fast data " << d;
#endif // OSC_DEBUG==1
    impl_->queue_.add(sender__,this,d.give_copy(),0,0);
    return true;
}

bool widget_t::fastdata_receive_event(const piw::data_nb_t &d,const piw::dataqueue_t &q)
{
    if(!d.is_null())
    {
#if OSC_DEBUG==1
    pic::logmsg() << "*** received fast event start " << name_ << ' ' << d;
#endif // OSC_DEBUG==1
        fastdata_t::ping(d.time(),q);
        return true;
    }

#if OSC_DEBUG==1
    pic::logmsg() << "*** received fast event end " << name_ << ' ' << d;
#endif // OSC_DEBUG==1
    return false;
}

widget_t::~widget_t()
{
    receiver_.send_slow(piw::makenull(piw::tsd_time()),piw::dataqueue_t());
    receiver_.close_fastdata();
    close_fastdata();
    lo_server_del_method(impl_->receiver_,name_,"f",method1__,this);
    lo_server_del_method(impl_->receiver_,name_,"",method2__,this);
    lo_server_del_method(impl_->receiver_,"/ping","",method3__,this);
    free((void *)name_);
}




service_broadcast_t::service_broadcast_t(language::oscserver_t::impl_t *impl) : impl_(impl)
{
    pic::logmsg() << "start service broadcast server";
    broadcast_server_ = lo_server_new(OSC_RENDEZVOUS_PORT, 0);
    request_addr_ = lo_server_add_method(broadcast_server_,"/eigend","",request_addr__,this);
    pic::logmsg() << "service broadcast server started";
}

service_broadcast_t::~service_broadcast_t()
{
    shutdown();
}

void service_broadcast_t::shutdown()
{
    if(impl_)
    {
        impl_ = 0;
        lo_server_del_method(broadcast_server_,"/eigend","",request_addr__,this);
        lo_server_free(broadcast_server_);
        pic::logmsg() << "service broadcast server shut down";
    }
}

void service_broadcast_t::receive()
{
    if(!impl_) return;

    lo_server_recv_noblock(broadcast_server_, 10);
}

int service_broadcast_t::request_addr__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data)
{
    if(!((service_broadcast_t*)user_data)->impl_) return 0;

    lo_address source_addr = lo_message_get_source(msg);

    // get the hostname and the port of the xmlrpc server
    lo_server broadcast_server = ((service_broadcast_t*)user_data)->broadcast_server_;

    // use the url of the broadcast server to get the hostname
    char *url = lo_server_get_url(broadcast_server);
    std::string xmlrpc_server_addr(url);
    free((void*)url);

    size_t addr_end = xmlrpc_server_addr.find_last_of(':');
    xmlrpc_server_addr = xmlrpc_server_addr.substr(10, addr_end-10+1) + ((service_broadcast_t*)user_data)->impl_->xmlrpc_server_port_;

    const char *sender_hostname = lo_address_get_hostname(source_addr);
    
    // has this come through the localhost loopback?
    int loopback = 0;
    if(strcmp(sender_hostname, "127.0.0.1")==0)
        loopback = 1;
    
    //pic::logmsg() << "request from address=" << sender_hostname << ":" << lo_address_get_port(source_addr) << " for xmlrpc server address replying address=" << xmlrpc_server_addr << " loopback=" << loopback;

    lo_send_from(source_addr,broadcast_server,LO_TT_IMMEDIATE,"/eigend","si",xmlrpc_server_addr.c_str(), loopback);
    
    return 0;
}



void language::oscserver_t::impl_t::add_recipient(lo_address a)
{
    int hole = -1;
    unsigned long long t = piw::tsd_time();

    for(unsigned i=0;i<recipients_.size();i++)
    {
        if(!recipients_[i])
        {
            hole = i;
            continue;
        }

        if(recipients_[i]->same_as(a))
        {
            recipients_[i]->time_=t;
            return;
        }
    }

    pic::logmsg() << "add recipient " << lo_address_get_hostname(a) << ":" << lo_address_get_port(a);

    recipient_t *b = new recipient_t(a,t);
    
    if(hole>=0)
    {
        recipients_[hole] = b;
    }
    else
    {
        recipients_.push_back(b);
    }

}

void language::oscserver_t::impl_t::send(const char *name,float v)
{
    // send widget value to all recipients (Stage clients)
    for(unsigned i=0;i<recipients_.size();i++)
    {
        if(!recipients_[i])
        {
            continue;
        }

        lo_send_from(recipients_[i]->address_,receiver_,LO_TT_IMMEDIATE,name,"f",v);
    }
}

void language::oscserver_t::impl_t::set_connected(unsigned index, bool is_connected)
{
    update_.lock();
    //pic::logmsg() << "-       OSC widget, set_connected: widget="<<index<<" "<<is_connected;
    if(index>0 && index<=widgets_.size())
    {
        index -= 1;
        if(widgets_[index])
            widgets_[index]->is_connected_ = is_connected;
        //else
            //pic::logmsg() << "-       OSC widget, set_connected: widget does not exist!";

    }
    //else
        //pic::logmsg() << "-       OSC widget, set_connected: widget index out of range";
    update_.unlock();
}


recipient_t::recipient_t(lo_address a, unsigned long long t): time_(t)
{
    address_ = lo_address_new_with_proto(lo_address_get_protocol(a),lo_address_get_hostname(a),lo_address_get_port(a));
}

recipient_t::~recipient_t()
{
    lo_address_free(address_);
}

bool recipient_t::same_as(lo_address a)
{
    if(lo_address_get_protocol(a) != lo_address_get_protocol(address_))
    {
        return false;
    }

    if(strcmp(lo_address_get_port(a),lo_address_get_port(address_)) != 0)
    {
        return false;
    }

    if(strcmp(lo_address_get_hostname(a),lo_address_get_hostname(address_)) != 0)
    {
        return false;
    }

    return true;
}
