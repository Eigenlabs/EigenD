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

#ifndef __PIA_SRC_BKERNEL__
#define __PIA_SRC_BKERNEL__

#include <pibelcanto/link.h>

#define PIE_BKERNEL_CLOCK_TICK_FREQ 1

typedef struct pie_bkaddr_s
{
    unsigned char space;
    unsigned char length;
    unsigned char data[BCTLINK_GROUP_SIZE];
} pie_bkaddr_t;

typedef struct pie_bkendpointops_s
{
    virtual ~pie_bkendpointops_s() {}
    virtual void close_callback(int hnd, void *ctx) = 0;
    virtual void data_callback(int hnd, void *ctx, const unsigned char *header, const void *payload, unsigned paylen) = 0;
} pie_bkendpointops_t;

typedef struct pie_bkdeviceops_s
{
    virtual ~pie_bkdeviceops_s() {}
    virtual void write_callback(void *ctx, const pie_bkaddr_t *grp, const unsigned char *header, const void *payload, unsigned paylen) = 0;
    virtual void addgroup_callback(void *ctx, const pie_bkaddr_t *grp) = 0;
    virtual void delgroup_callback(void *ctx, const pie_bkaddr_t *grp) = 0;
} pie_bkdeviceops_t;

class pie_bkernel_t
{
    public:
        class impl_t;

    public:
        pie_bkernel_t(const pie_bkaddr_t *id);
        ~pie_bkernel_t();
        int pie_bkopen(pie_bkendpointops_t *ops, void *ctx, const pie_bkaddr_t *group, int promisc);
        void *pie_bkcontext(int ep);
        int pie_bkclose(int ep, void **arg);
        int pie_bkgroupname(int ep, pie_bkaddr_t *);
        int pie_bkwrite(int ep, const void *payload, unsigned paylen);
        unsigned pie_bkheadroom(void);
        void pie_bkgetaddr(pie_bkaddr_t *);
        void pie_bktick(void);
        int pie_bkadddevice(void *ctx, const pie_bkaddr_t *id, unsigned headroom, pie_bkdeviceops_t *ops);
        void pie_bkremovedevice(int dev, int unhook);
        int pie_bkfinddevice(void *ctx);
        int pie_bkfinddevice2(pie_bkdeviceops_t *, int (*)(void *ctx, void *arg), void *arg);
        void *pie_bkfinddevice3(pie_bkdeviceops_t *, int (*)(void *ctx, void *arg), void *arg);
        void pie_bkdata(int dev, const unsigned char *header, const void *payload, unsigned paylen);
        static void pie_bkaddrhash(const pie_bkaddr_t *addr, void *hash, unsigned hlen);
        static void pie_bin2hex(const void *bin, unsigned binlen, char *hex);
    private:
        impl_t *impl_;
};

#endif
