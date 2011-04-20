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


#include <picross/pic_log.h>
#include <picross/pic_ilist.h>
#include <picross/pic_time.h>
#include <picross/pic_thread.h>
#include <picross/pic_stl.h>
#include <piagent/pia_udpnet.h>

#include <string.h>
#include <stdlib.h>

#include <map>

#include "pia_bkernel.h"
#include "pia_bclock.h"

#define PIA_WORKER_PRIORITY 0
#define PIA_TIMER_PRIORITY 0

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <iwlib.h>

#include "pia_data.h"
#include "pia_glue.h"

#include <errno.h>

#define PORTBASE_LOCAL 55555
#define PORTBASE_ETHER 56555
#define PORTBASE(loop) ((loop)?PORTBASE_LOCAL:PORTBASE_ETHER)

namespace
{
    struct checker_t
    {
        virtual ~checker_t() {}
        virtual bool check(unsigned spc,unsigned long, unsigned short,int *d) = 0;
    };

    struct socket_t : pic::nocopy_t
    {
        socket_t(int f,int t, int p)
        {
            if((fd=socket(f,t,p))<0)
            {
                pic::msg() << "can't open socket" << pic::hurl;
            }
        }

        ~socket_t()
        {
            close(fd);
        }

        int fd;
    };

    struct send_socket_t
    {
        send_socket_t(unsigned spc, unsigned long local,bool loop): socket_(AF_INET,SOCK_DGRAM,0), spc_(spc), local_(local), loop_(loop)
        {
            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);
 
            memset((char *) &addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(0);
            addr.sin_addr.s_addr  = local;
 
            if(bind(socket_.fd, (struct sockaddr*)&addr, sizeof(addr)))
            {
                pic::msg() << "Can't bind sender socket" << pic::hurl;
            }

            getsockname(socket_.fd, (struct sockaddr*)&addr, &addrlen);
            port_ = addr.sin_port;

            u_char loopback=loop?1:0;
            if(setsockopt(socket_.fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopback, sizeof(loopback)) < 0)
            {
                pic::msg() << "Can't setup looping" << pic::hurl;
            }

            u_char ttl=loop?0:1;
            if(setsockopt(socket_.fd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) < 0)
            {
                pic::msg() << "Can't setup mcast ttl" << pic::hurl;
            }

            int ssz = BCTLINK_MAXPACKET;
            if(setsockopt(socket_.fd, SOL_SOCKET, SO_SNDBUF, (char *)&ssz, sizeof(ssz)) < 0)
            {
                pic::msg() << "Can't set sndbuf size" << pic::hurl;
            }

            struct in_addr mca;
            memset(&mca,0,sizeof(mca));
            mca.s_addr = local;

            if(setsockopt(socket_.fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&mca, sizeof(mca)) < 0)
            {
                pic::msg() << "Can't setup mcast if" << pic::hurl;
            }
        }

        ~send_socket_t()
        {
        }

        void send(unsigned long addr, const unsigned char *data, unsigned len)
        {
             struct sockaddr_in group;

             memset(&group,0,sizeof(group));
             group.sin_family = AF_INET;
             group.sin_addr.s_addr = addr;
             group.sin_port = htons(PORTBASE(loop_)+spc_);

             ssize_t s = sendto(socket_.fd, (char *)data, len, 0, (struct sockaddr*)&group, sizeof(group));
             if(s != (int)len)
             {
                pic::msg() << "Can't send multicast data on " << socket_.fd << ':' << (void *)this << ": " << sys_errlist[errno] << pic::log;
             }

        }

        unsigned short port() { return port_; }

        socket_t socket_;
        unsigned spc_;
        unsigned long local_;
        unsigned short port_;
        bool loop_;
    };

    struct recv_socket_t
    {
        recv_socket_t(unsigned spc, unsigned long local, bool loop): socket_(AF_INET,SOCK_DGRAM,0), spc_(spc), local_(local)
        {
            int reuse=1;
            struct sockaddr_in addr;
 
            if(setsockopt(socket_.fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
            {
                pic::msg() << "Can't setup mcast socket" << pic::hurl;
            }

            int rsz = BCTLINK_MAXPACKET;
            if(setsockopt(socket_.fd, SOL_SOCKET, SO_RCVBUF, (char *)&rsz, sizeof(rsz)) < 0)
            {
                pic::msg() << "Can't set rcvbuf size" << pic::hurl;
            }

            memset((char *) &addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORTBASE(loop)+spc_);
            addr.sin_addr.s_addr = INADDR_ANY;
 
            if(bind(socket_.fd, (struct sockaddr*)&addr, sizeof(addr)))
            {
                pic::msg() << "Can't bind mcast socket" << pic::hurl;
            }
        }

        ~recv_socket_t()
        {
        }

        int fd()
        {
            return socket_.fd;
        }

        void join(unsigned long addr)
        {
            struct ip_mreq group;
            memset(&group,0,sizeof(group));
            pic::lckmap_t<unsigned long,unsigned>::type::iterator i;

            if((i=subscriptions_.find(addr)) != subscriptions_.end())
            {
                if(i->second>0)
                {
                    i->second++;
                    return;
                }

                i->second=1;
            }
            else
            {
                subscriptions_.insert(std::make_pair(addr,1));
            }

            group.imr_multiaddr.s_addr = addr;
            group.imr_interface.s_addr = local_;

            if(setsockopt(socket_.fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
            {
                pic::msg() << "Can't join mcast socket: " << addr << ":" << sys_errlist[errno] << pic::log;
            }
       }

        void leave(unsigned long addr)
        {
            struct ip_mreq group;
            pic::lckmap_t<unsigned long,unsigned>::type::iterator i;

            if((i=subscriptions_.find(addr)) == subscriptions_.end() || i->second==0)
            {
                pic::msg() << "invalid mcast unsubscription" << pic::log;
                return;
            }

            if(--i->second > 0)
            {
                return;
            }

            group.imr_multiaddr.s_addr = addr;
            group.imr_interface.s_addr = local_;

            if(setsockopt(socket_.fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
            {
                pic::msg() << "Can't leave mcast socket: " << addr << ":" << sys_errlist[errno] << pic::log;
            }
        }

        int recv(unsigned long *addr, unsigned short *port, unsigned char *data, unsigned len)
        {
            int l;
            struct sockaddr_in a;
            memset(&a,0,sizeof(a));
            a.sin_family=AF_INET;
            socklen_t al = sizeof(a);

            if((l=recvfrom(socket_.fd, (char *)data, len, MSG_WAITALL|MSG_DONTWAIT, (struct sockaddr *)&a, &al)) < 0)
            {
                if(errno==EAGAIN)
                {
                    return 0;
                }

                pic::msg() << "error receiving multicast data" << pic::hurl;
            }

            *addr = a.sin_addr.s_addr;
            *port = a.sin_port;
            return l;
        }

        void network(pie_bkernel_t *kernel,checker_t *checker, int device)
        {
            unsigned char buffer[BCTLINK_MAXPACKET];
            int len;
            unsigned long a; unsigned short p;
            
            while((len=recv(&a,&p,buffer,sizeof(buffer)))>0)
            {
                if(!checker->check(spc_,a,p,&device))
                {
                    //unsigned char *x = (unsigned char *)&a;
                    //pic::logmsg() << "rejected message from " <<
                    //    (unsigned)x[0] << "." <<
                    //    (unsigned)x[1] << "." <<
                    //    (unsigned)x[2] << "." <<
                    //    (unsigned)x[3] << ":" << p;
                    continue;
                }

                if(len>BCTLINK_HEADER)
                {
                    len-=BCTLINK_HEADER;
                    kernel->pie_bkdata(device,buffer,&buffer[BCTLINK_HEADER],len);
                }
            }
        }

        socket_t socket_;
        unsigned spc_;
        unsigned long local_;
        pic::lckmap_t<unsigned long,unsigned>::type subscriptions_;
    };

    struct interface_t: pie_bkdeviceops_t
    {
        interface_t(pie_bkernel_t *kernel,unsigned long ifaddr, bool loop): local_(ifaddr), device_(-1), loop_(loop),
             rsocket0_(0,ifaddr,loop), rsocket1_(1,ifaddr,loop), rsocket2_(2,ifaddr,loop), rsocket3_(3,ifaddr,loop), rsocket4_(4,ifaddr,loop), rsocket5_(5,ifaddr,loop), 
             ssocket0_(0,ifaddr,loop), ssocket1_(1,ifaddr,loop), ssocket2_(2,ifaddr,loop), ssocket3_(3,ifaddr,loop), ssocket4_(4,ifaddr,loop), ssocket5_(5,ifaddr,loop),
             kernel_(kernel)
        {
            if((device_ = kernel_->pie_bkadddevice(this,0,0,this)) < 0)
            {
                pic::msg() << "Can't add interface bkernel" << pic::hurl;
            }
        }

        ~interface_t()
        {
            //pic::logmsg() << "removing interface";
            kernel_->pie_bkremovedevice(device_,false);
        }

        int bk()
        {
            return device_;
        }

        static unsigned long hash(const pie_bkaddr_t *grp)
        {
            unsigned long a = 0;
            unsigned char h[1];

            a |= 225;
            a <<= 8;
            a |= 0;
            a <<= 8;
            a |= grp->space;
            a <<= 8;

            pie_bkernel_t::pie_bkaddrhash(grp,&h,1);
            a |= h[0]%20;

            return htonl(a);
        }

        void write_callback(void *ctx, const pie_bkaddr_t *grp, const unsigned char *hdr, const void *data, unsigned len)
        {
            interface_t *intf = (interface_t *)ctx;
            unsigned long addr = hash(grp);

            unsigned char buffer[BCTLINK_MAXPACKET];
            memcpy(buffer,hdr,BCTLINK_HEADER);
            memcpy(&buffer[BCTLINK_HEADER],data,len);

            switch(grp->space)
            {
                case 0: intf->ssocket0_.send(addr,buffer,BCTLINK_HEADER+len); break;
                case 1: intf->ssocket1_.send(addr,buffer,BCTLINK_HEADER+len); break;
                case 2: intf->ssocket2_.send(addr,buffer,BCTLINK_HEADER+len); break;
                case 3: intf->ssocket3_.send(addr,buffer,BCTLINK_HEADER+len); break;
                case 4: intf->ssocket4_.send(addr,buffer,BCTLINK_HEADER+len); break;
                default: intf->ssocket5_.send(addr,buffer,BCTLINK_HEADER+len); break;
            }
        }

        void addgroup_callback(void *ctx, const pie_bkaddr_t *grp)
        {
            interface_t *intf = (interface_t *)ctx;
            unsigned long addr = hash(grp);

            switch(grp->space)
            {
                case 0: intf->rsocket0_.join(addr); break;
                case 1: intf->rsocket1_.join(addr); break;
                case 2: intf->rsocket2_.join(addr); break;
                case 3: intf->rsocket3_.join(addr); break;
                case 4: intf->rsocket4_.join(addr); break;
                default: intf->rsocket5_.join(addr); break;
            }
        }

        void delgroup_callback(void *ctx, const pie_bkaddr_t *grp)
        {
            interface_t *intf = (interface_t *)ctx;
            unsigned long addr = hash(grp);
            
            switch(grp->space)
            {
                case 0: intf->rsocket0_.leave(addr); break;
                case 1: intf->rsocket1_.leave(addr); break;
                case 2: intf->rsocket2_.leave(addr); break;
                case 3: intf->rsocket3_.leave(addr); break;
                case 4: intf->rsocket4_.leave(addr); break;
                default: intf->rsocket5_.leave(addr); break;
            }
        }

        unsigned short sp(unsigned spc)
        {
            switch(spc)
            {
                case 0:  return ssocket0_.port();
                case 1:  return ssocket1_.port();
                case 2:  return ssocket2_.port();
                case 3:  return ssocket3_.port();
                case 4:  return ssocket4_.port();
                default: return ssocket5_.port();
            }
        }

        void populate_fd(fd_set *r)
        {
            FD_SET(rsocket0_.fd(),r);
            FD_SET(rsocket1_.fd(),r);
            FD_SET(rsocket2_.fd(),r);
            FD_SET(rsocket3_.fd(),r);
            FD_SET(rsocket4_.fd(),r);
            FD_SET(rsocket5_.fd(),r);
        }

        void process_fd(fd_set *r, checker_t *checker)
        {
            if(FD_ISSET(rsocket0_.fd(),r)) rsocket0_.network(kernel_,checker,device_);
            if(FD_ISSET(rsocket1_.fd(),r)) rsocket1_.network(kernel_,checker,device_);
            if(FD_ISSET(rsocket2_.fd(),r)) rsocket2_.network(kernel_,checker,device_);
            if(FD_ISSET(rsocket3_.fd(),r)) rsocket3_.network(kernel_,checker,device_);
            if(FD_ISSET(rsocket4_.fd(),r)) rsocket4_.network(kernel_,checker,device_);
            if(FD_ISSET(rsocket5_.fd(),r)) rsocket5_.network(kernel_,checker,device_);
        }

        unsigned short sp0() { return ssocket0_.port(); }
        unsigned short sp1() { return ssocket1_.port(); }
        unsigned short sp2() { return ssocket2_.port(); }
        unsigned short sp3() { return ssocket3_.port(); }
        unsigned short sp4() { return ssocket4_.port(); }
        unsigned short sp5() { return ssocket5_.port(); }
        unsigned long ip() { return local_; }

        unsigned long local_;
        int device_;
        bool loop_;

        recv_socket_t rsocket0_;
        recv_socket_t rsocket1_;
        recv_socket_t rsocket2_;
        recv_socket_t rsocket3_;
        recv_socket_t rsocket4_;
        recv_socket_t rsocket5_;

        send_socket_t ssocket0_;
        send_socket_t ssocket1_;
        send_socket_t ssocket2_;
        send_socket_t ssocket3_;
        send_socket_t ssocket4_;
        send_socket_t ssocket5_;

        pie_bkernel_t *kernel_;
    };

    struct endpoint_t: pie_bkendpointops_t
    {
        endpoint_t(pie_bkernel_t *kernel, unsigned space,const char *name,bool promisc): callback_(0), kernel_(kernel)
        {
            pie_bkaddr_t group;

            //pic::printmsg() << "opening " << name << " on bk " << (void *)kernel_;

            group.space = space;
            group.length = strlen(name);
            memcpy(group.data,name,group.length);

            if((endpoint_=kernel_->pie_bkopen(this,(void *)this,&group,promisc?1:0)) < 0)
            {
                pic::msg() << "Can't open socket" << pic::hurl;
            }
        }

        ~endpoint_t()
        {
            kernel_->pie_bkclose(endpoint_,0);
        }

        int write(const void *buffer, unsigned len)
        {
            int r = kernel_->pie_bkwrite(endpoint_,buffer,len);
            return r;
        }

        void callback(void (*cb)(void *ctx, const unsigned char *, unsigned), void *ctx)
        {
            pic::mutex_t::guard_t g(mutex_);
            callback_=cb;
            ctx_=ctx;
        }

        void close_callback(int ep, void *h_)
        {
        }

        void data_callback(int ep, void *h_, const unsigned char *hdr, const void *data, unsigned len)
        {
            endpoint_t *h = (endpoint_t *)h_;

            try
            {
                pic::mutex_t::guard_t g(h->mutex_);

                if(h->callback_)
                {
                    h->callback_(h->ctx_,(const unsigned char *)data,len);
                }
            }
            PIA_CATCHLOG_PRINT()
        }

        int endpoint_;
        pic::mutex_t mutex_;
        void (*callback_)(void *,const unsigned char *, unsigned);
        void *ctx_;
        pie_bkernel_t *kernel_;
    };

    struct pia_bkernel_clock_thread_t: public pic::thread_t
    {
        pia_bkernel_clock_thread_t(pie_clock_t *clock): pic::thread_t(PIA_TIMER_PRIORITY), clock_(clock), stop_(false)
        {
        }

        void thread_main()
        {
            while(!stop_)
            {
                //pic_microsleep(1000000/PIE_CLOCK_TICK_FREQ);
                pic_microsleep(1000000);
                //pie_clock_tick();
            }
        }

        pie_clock_t *clock_;
        bool stop_;
    };

    struct pia_bkernel_proto_thread_t: public pic::thread_t
    {
        pia_bkernel_proto_thread_t(pie_bkernel_t *kernel): pic::thread_t(PIA_TIMER_PRIORITY), kernel_(kernel), stop_(false)
        {
        }

        void thread_main()
        {
            while(!stop_)
            {
                //pic_microsleep(1000000/PIE_BKERNEL_CLOCK_TICK_FREQ);
                pic_microsleep(1000000);
                kernel_->pie_bktick();
            }
        }

        pie_bkernel_t *kernel_;
        bool stop_;
    };

    struct bkernel_t: pie_bkernel_t, pie_clock_t
    {
        bkernel_t(pic::nballocator_t *allocator, bool clock): pie_bkernel_t(0), pie_clock_t(this), cthread_(this), pthread_(this)
        {
            pthread_.run();
            cthread_.run();
        }

        ~bkernel_t()
        {
            cthread_.stop_=true;
            pthread_.stop_=true;
            pthread_.wait();
            cthread_.wait();
        }

        pia_bkernel_clock_thread_t cthread_;
        pia_bkernel_proto_thread_t pthread_;

    };

    struct netbase_t: pic::thread_t, checker_t
    {
        netbase_t(pic::nballocator_t *a, bool clock): pic::thread_t(0), allocator_(a), shutdown_(false), bkernel_(allocator_,clock), local_(false), loopback_(&bkernel_,inet_addr("127.0.0.1"),true)
        {
            if(getenv("PI_FULLNET")==0)
            {
                pic::logmsg() << "warning: networking disabled";
                local_ = true;
            }
        }

        ~netbase_t()
        {
        }

        bool check(unsigned spc, unsigned long a, unsigned short p, int *d)
        {
            std::map<std::string,std::pair<unsigned long,interface_t *> >::iterator i;

            bool remotehost = true;

            if(a==loopback_.local_)
                remotehost = false;

            for(i=interfaces_.begin(); i!=interfaces_.end(); i++)
                if(a==i->second.second->local_)
                    remotehost = false;

            if(remotehost)
                return true;

            bool remoteproc = true;

            if(p==loopback_.sp(spc))
                remoteproc = false;

            for(i=interfaces_.begin(); i!=interfaces_.end(); i++)
                if(p==i->second.second->sp(spc))
                    remoteproc = false;

            if(remoteproc)
            {
                *d = loopback_.bk();
                return true;
            }

            return false;
        }

        void shutdown()
        {
            //pic::logmsg() << "shutting down network";
            shutdown_ = true;
            wait();
            //pic::logmsg() << "shut down network";
        }

        virtual void populate_monitor(fd_set *r)
        {
        }

        virtual bool process_monitor(fd_set *r)
        {
            return false;
        }

        void thread_init()
        {
            if(!local_)
                scan();
        }

        void thread_main()
        {
            fd_set r;
            struct timeval tv;
            int rv;

            while(!shutdown_)
            {
                tv.tv_sec=0;
                tv.tv_usec=250000;

                FD_ZERO(&r);

                populate_fd(&r);
                if(!local_)
                    populate_monitor(&r);

                if((rv=::select(FD_SETSIZE,&r,0,0,&tv))<0)
                {
                    pic::msg() << "error in select: " << rv << ',' << sys_errlist[errno] << pic::hurl;
                }

                if(local_ || !process_monitor(&r))
                {
                    process_fd(&r);
                }
            }
        }

        void populate_fd(fd_set *r)
        {
            std::map<std::string,std::pair<unsigned long,interface_t *> >::iterator i;

            for(i=interfaces_.begin(); i!=interfaces_.end(); i++)
            {
                i->second.second->populate_fd(r);
            }

            loopback_.populate_fd(r);
        }

        void process_fd(fd_set *r)
        {
            std::map<std::string,std::pair<unsigned long,interface_t *> >::iterator i;

            for(i=interfaces_.begin(); i!=interfaces_.end(); i++)
            {
                i->second.second->process_fd(r, this);
            }

            loopback_.process_fd(r, this);
        }


        interface_t *create_intf(unsigned long addr)
        {
            try
            {
                return new interface_t(&bkernel_,addr,false);
            }
            catch(...)
            {
                return 0;
            }
        }

        void destroy_intf(interface_t *intf)
        {
            delete intf;
        }

        void scan()
        {
            pic::logmsg() << "scanning interfaces";
            struct ifaddrs *ifap;
            std::map<std::string,std::pair<unsigned long,interface_t *> >::iterator i,deadi;
            std::map<std::string,std::pair<unsigned long, interface_t *> > dead;
            unsigned long a;

            for(i=interfaces_.begin(); i!=interfaces_.end(); i++)
            {
                dead.insert(*i);
            }

            if(getifaddrs(&ifap)<0)
            {
                pic::logmsg() << "can't get list of current interfaces";
                return;
            }
            
            while(ifap)
            {
                if(!ifap->ifa_addr || ifap->ifa_addr->sa_family != AF_INET)
                {
                    ifap=ifap->ifa_next;
                    continue;
                }

                std::string n(ifap->ifa_name);
                a = interface_filter(n.c_str());

                if(!a)
                {
                    ifap=ifap->ifa_next;
                    continue;
                }

                i=interfaces_.find(n);

                if(i!=interfaces_.end() && i->second.first!=a)
                {
                    pic::logmsg() << "changed interface " << n;
                    dead.erase(n);
                    destroy_intf(i->second.second);
                    interfaces_.erase(i);
                    i=interfaces_.end();
                }

                if(i==interfaces_.end())
                {
                    pic::logmsg() << "new interface " << n;
                    interfaces_.insert(std::make_pair(n,std::make_pair(a,create_intf(a))));
                    ifap=ifap->ifa_next;
                    continue;
                }

                pic::logmsg() << "unchanged interface " << n;
                dead.erase(n);
                ifap=ifap->ifa_next;
            }

            freeifaddrs(ifap);

            for(deadi=dead.begin(); deadi!=dead.end(); deadi++)
            {
                pic::logmsg() << "dead interface " << deadi->first;
                destroy_intf(deadi->second.second);
                interfaces_.erase(deadi->first);
            }
        }

        virtual unsigned long interface_filter(const char *name)
        {
            return 0;
        }

        pic::nballocator_t *allocator_;
        bool shutdown_;
        bkernel_t bkernel_;
        bool local_;
        interface_t loopback_;
        std::map<std::string,std::pair<unsigned long, interface_t *> > interfaces_;
    };
};


struct pia::udpnet_t::impl_t: netbase_t
{
    impl_t(pic::nballocator_t *a, bool clock): netbase_t(a,clock),kevsock_(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE),statsock_(AF_INET,SOCK_DGRAM,0)
    {
        run();
        setup_kev();
    }

    ~impl_t()
    {
        shutdown();
    }

    void setup_kev()
    {
        struct sockaddr_nl sa;

        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        sa.nl_groups = RTMGRP_IPV4_IFADDR;

        if(bind(kevsock_.fd, (struct sockaddr*)&sa, sizeof(sa))<0)
        {
            pic::msg() << "can't establish kernel event filter" << pic::hurl;
        }

    }

    virtual void populate_monitor(fd_set *r)
    {
        FD_SET(kevsock_.fd,r);
    }

    virtual bool process_monitor(fd_set *r)
    {
        if(FD_ISSET(kevsock_.fd,r))
        {
            unsigned char buffer[4096];
            recv(kevsock_.fd,buffer,sizeof(buffer),0);
            scan();
            return true;
        }

        return false;
    }

    virtual unsigned long interface_filter(const char *name)
    {
        struct ifreq ifr;
        unsigned long addr;
        unsigned short flags;
        struct iwreq iwr;

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

        if (ioctl(statsock_.fd, SIOCGIFFLAGS, (caddr_t)&ifr) < 0)
        {
            pic::logmsg() << name << ": can't get flags";
            return 0;
        }

        flags = ifr.ifr_flags;

        if((flags&IFF_LOOPBACK)!=0)
        {
            pic::logmsg() << name << ": is loopback";
            return 0;
        }

        if((flags&IFF_UP)==0)
        {
            pic::logmsg() << name << ": is not running";
            return 0;
        }

        if((flags&IFF_MULTICAST)==0)
        {
            pic::logmsg() << name << ": is not multicast";
            return 0;
        }

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

        if (ioctl(statsock_.fd, SIOCGIFADDR, (caddr_t)&ifr) < 0)
        {
            pic::logmsg() << name << ": can't get address";
            return 0;
        }

        addr = ((struct sockaddr_in *)(&(ifr.ifr_addr)))->sin_addr.s_addr;

        pic::logmsg() << name << std::hex << ": " << addr;

        memset(&iwr, 0, sizeof(iwr));
        strncpy(iwr.ifr_name, name, sizeof(iwr.ifr_name));

        if(ioctl(statsock_.fd, SIOCGIWNAME, &iwr)>=0)
        {
            pic::logmsg() << name << ": wireless";
            return 0;
        }

        return addr;
    }

    socket_t kevsock_;
    socket_t statsock_;
};

pia::udpnet_t::udpnet_t(pic::nballocator_t *a, bool clock)
{
    pia_logguard_t guard(0,a);
	impl_=new impl_t(a,clock);
}

pia::udpnet_t::~udpnet_t()
{
    pia_logguard_t guard(0,impl_->allocator_);
	delete impl_;
}

void *pia::udpnet_t::network_open(unsigned space,const char *name, bool promisc)
{
    pia_logguard_t guard(0,impl_->allocator_);
    endpoint_t *h = new endpoint_t(&impl_->bkernel_,space,name,promisc);
    return (void *)h;
}

void pia::udpnet_t::network_close(void *handle_)
{
    pia_logguard_t guard(0,impl_->allocator_);
    endpoint_t *h = (endpoint_t *)handle_;
    delete h;
}

int pia::udpnet_t::network_write(void *handle_, const void *buffer, unsigned len)
{
    pia_logguard_t guard(0,impl_->allocator_);
    endpoint_t *h = (endpoint_t *)handle_;
    return h->write(buffer,len);
}

int pia::udpnet_t::network_callback(void *handle_, void (*cb)(void *ctx, const unsigned char *, unsigned), void *ctx)
{
    pia_logguard_t guard(0,impl_->allocator_);
    endpoint_t *h = (endpoint_t *)handle_;
    h->callback(cb,ctx);
    return 0;
}

int pia::udpnet_t::network_time(void *handle_, unsigned long long *time)
{
    pia_logguard_t guard(0,impl_->allocator_);
    *time = pic_microtime();
    return 1;
}
