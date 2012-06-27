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

#ifndef __PIBELCANTO_PLUGIN__
#define __PIBELCANTO_PLUGIN__

#include <picross/pic_thread.h>
#include "state.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <picross/pic_stdint.h>

/**
 * @defgroup plg_clock clock constants
 * @ingroup plg
 * @{
 */

#define PLG_CLOCK_BUFFER_SIZE          4096
#define PLG_CLOCK_BUFFER_SIZE_DEFAULT  512

/**
 * @}
 */

/**
 * @defgroup plg_server server flags
 * @ingroup plg
 * @{
 */

#define PLG_SERVER_RO              BCTVTYPE_READONLY    /**< server node is read only */
#define PLG_SERVER_FAST            BCTVTYPE_FAST        /**< server node has fast data */
#define PLG_SERVER_RTRANSIENT      BCTVTYPE_RTRANSIENT  /**< server and subtree should be ignored by state */
#define PLG_SERVER_TRANSIENT       BCTVTYPE_TRANSIENT   /**< server should be ignored by state */
#define PLG_SERVER_LIST            BCTVTYPE_LIST        /**< server is dynamic list */
#define PLG_SERVER_CTL             BCTVTYPE_CTL         /**< server carries control plumbing */

/**
 * @}
 */

#define PLG_STATE_CLOSED           0
#define PLG_STATE_ATTACHED         1
#define PLG_STATE_OPENED           2
#define PLG_STATE_DETACHED         3

/**
 * @defgroup plg_client client flags
 * @ingroup plg
 * @{
 */

#define PLG_CLIENT_SYNC            1                    /**< client needs sync callback */
#define PLG_CLIENT_CLOCK           2                    /**< client needs clock callback */

/**
 * @}
 */

/**
 * @defgroup plg_fastdata fastdata flags
 * @ingroup plg
 * @{
 */

#define PLG_FASTDATA_SENDER        1                    /**< fastdata is data source */

/**
 * @}
 */

typedef struct bct_agent_s bct_agent_t;
typedef struct bct_index_s bct_index_t;
typedef struct bct_rpcserver_s bct_rpcserver_t;
typedef struct bct_rpcclient_s bct_rpcclient_t;
typedef struct bct_client_s bct_client_t;
typedef struct bct_server_s bct_server_t;
typedef struct bct_thing_s bct_thing_t;
typedef struct bct_window_s bct_window_t;
typedef struct bct_fastdata_s bct_fastdata_t;
typedef struct bct_clocksink_s bct_clocksink_t;
typedef struct bct_clocksource_s bct_clocksource_t;
typedef struct bct_clockdomain_s bct_clockdomain_t;

typedef struct bct_client_host_ops_s bct_client_host_ops_t;
typedef struct bct_client_plug_ops_s bct_client_plug_ops_t;
typedef struct bct_server_host_ops_s bct_server_host_ops_t;
typedef struct bct_server_plug_ops_s bct_server_plug_ops_t;
typedef struct bct_index_host_ops_s bct_index_host_ops_t;
typedef struct bct_index_plug_ops_s bct_index_plug_ops_t;
typedef struct bct_thing_host_ops_s bct_thing_host_ops_t;
typedef struct bct_thing_plug_ops_s bct_thing_plug_ops_t;
typedef struct bct_window_host_ops_s bct_window_host_ops_t;
typedef struct bct_window_plug_ops_s bct_window_plug_ops_t;
typedef struct bct_fastdata_host_ops_s bct_fastdata_host_ops_t;
typedef struct bct_fastdata_plug_ops_s bct_fastdata_plug_ops_t;
typedef struct bct_clocksink_host_ops_s bct_clocksink_host_ops_t;
typedef struct bct_clocksink_plug_ops_s bct_clocksink_plug_ops_t;
typedef struct bct_clocksource_host_ops_s bct_clocksource_host_ops_t;
typedef struct bct_clocksource_plug_ops_s bct_clocksource_plug_ops_t;
typedef struct bct_clockdomain_host_ops_s bct_clockdomain_host_ops_t;
typedef struct bct_clockdomain_plug_ops_s bct_clockdomain_plug_ops_t;
typedef struct bct_rpcserver_host_ops_s bct_rpcserver_host_ops_t;
typedef struct bct_rpcserver_plug_ops_s bct_rpcserver_plug_ops_t;
typedef struct bct_rpcclient_host_ops_s bct_rpcclient_host_ops_t;
typedef struct bct_rpcclient_plug_ops_s bct_rpcclient_plug_ops_t;

typedef struct bct_entity_ops_s bct_entity_ops_t;
typedef bct_entity_ops_t **bct_entity_t;

typedef struct bct_data_ops_s bct_data_ops_t;
typedef struct bct_datahdr_s bct_datahdr_t;
typedef struct bct_data_s *bct_data_t;

typedef struct bct_dataqueue_host_ops_s bct_dataqueue_host_ops_t;
typedef struct bct_dataqueue_s *bct_dataqueue_t;

/**
 * A note on method annotations:
 *
 * Unannotated means that the method can only be called from the
 * slow thread.  Otherwise, as indicated.
 */

struct bct_datahdr_s
{
    unsigned type;
    unsigned scalar_len;
    unsigned vector_len;
    unsigned wire_len;
    unsigned long long time;
    float ubound;
    float lbound;
    float rest;
    unsigned nb_mode;
};


/**
 * General reference counted, immutable data object.
 */
struct bct_data_ops_s
{
    void (*data_free)(const bct_data_t); /* any thread */
    const unsigned char *(*data_data)(const bct_data_t); /* any thread */
    const float *(*data_vector)(const bct_data_t); /* any thread */
    const unsigned char *(*data_wiredata)(const bct_data_t); /* any thread */
};

struct bct_data_s
{
    bct_data_ops_t *host_ops;
    uint32_t count;
    bct_datahdr_t host_hdr;
    #ifdef DEBUG_DATA_ATOMICITY
    pic_threadid_t tid;
    bool nb_usage;
    #endif
};

typedef bct_data_s *bct_data_t;

#define bct_data_hdr(bc)       ((bc)?(&((bc)->host_hdr)):(0))
#define bct_data_veclen(bc)    ((bc)?(bct_data_hdr(bc)->vector_len):(0))
#define bct_data_lbound(bc)    ((bc)?(bct_data_hdr(bc)->lbound):(0.0))
#define bct_data_ubound(bc)    ((bc)?(bct_data_hdr(bc)->ubound):(0.0))
#define bct_data_rest(bc)      ((bc)?(bct_data_hdr(bc)->rest):(0.0))
#define bct_data_time(bc)      ((bc)?(bct_data_hdr(bc)->time):(0ULL))
#define bct_data_type(bc)      ((bc)?((bct_data_hdr(bc)->type)&0x0f):(BCTVTYPE_NULL))
#define bct_data_units(bc)     ((bc)?((bct_data_hdr(bc)->type)&0xf0):(BCTVTYPE_NULL))
#define bct_data_len(bc)       ((bc)?(bct_data_hdr(bc)->scalar_len):(0))
#define bct_data_wirelen(bc)   ((bc)?(bct_data_hdr(bc)->wire_len):(0))
#define bct_data_free(bc)      do { (bc)->host_ops->data_free((bc)); } while(0)
#define bct_data_data(bc)      ((bc)?((bc)->host_ops->data_data((bc))):((const unsigned char *)""))
#define bct_data_vector(bc)    ((bc)?((bc)->host_ops->data_vector((bc))):(0))
#define bct_data_wiredata(bc)  ((bc)?((bc)->host_ops->data_wiredata((bc))):((const unsigned char *)""))
#define bct_data_count(bc)     ((bc)?((bc)->count):(0))
#define bct_data_nb_mode(bc)   ((bc)?(bct_data_hdr(bc)->nb_mode):(0))

struct bct_dataqueue_host_ops_s
{
    void (*dataqueue_free)(const bct_dataqueue_t);
    int (*dataqueue_write)(const bct_dataqueue_t, bct_data_t); /* fast thread */
    int (*dataqueue_read)(const bct_dataqueue_t, bct_data_t *, unsigned long long *index, unsigned long long time); /* fast thread */
    /* earliest after or equal to time */
    int (*dataqueue_earliest)(const bct_dataqueue_t, bct_data_t *, unsigned long long *index, unsigned long long time); /* fast thread */
    /* latest before or equal to time */
    int (*dataqueue_latest)(const bct_dataqueue_t, bct_data_t *, unsigned long long *index, unsigned long long time); /* fast thread */
    void (*dataqueue_dump)(const bct_dataqueue_t,int); /* fast thread */
    bct_data_t (*dataqueue_current)(const bct_dataqueue_t); /* fast thread */
};

struct bct_dataqueue_s
{
    bct_dataqueue_host_ops_t *host_ops;
    uint32_t count;
};

typedef bct_dataqueue_s *bct_dataqueue_t;

#define bct_dataqueue_free(bc)           ((bc)->host_ops->dataqueue_free((bc)))
#define bct_dataqueue_write(bc,d)        ((bc)->host_ops->dataqueue_write((bc),(d)))
#define bct_dataqueue_read(bc,d,i,t)     ((bc)->host_ops->dataqueue_read((bc),(d),(i),(t)))
#define bct_dataqueue_earliest(bc,d,i,t) ((bc)->host_ops->dataqueue_earliest((bc),(d),(i),(t)))
#define bct_dataqueue_latest(bc,d,i,t)   ((bc)->host_ops->dataqueue_latest((bc),(d),(i),(t)))
#define bct_dataqueue_dump(bc,f)         ((bc)->host_ops->dataqueue_dump((bc),(f)))
#define bct_dataqueue_current(bc)        ((bc)->host_ops->dataqueue_current((bc)))
#define bct_dataqueue_count(bc)          ((bc)?((bc)->count):(0))

/**
 * The context or environment for an agent, in which its clients,
 * servers, indices, etc. live.
 */
struct bct_entity_ops_s
{
    unsigned entity_version;
    int (*entity_server)(bct_entity_t, const char *n, bct_server_t *s);
    int (*entity_client)(bct_entity_t, const char *n, bct_client_t *s, int fast);
    int (*entity_index)(bct_entity_t, const char *n, bct_index_t *s);
    int (*entity_fastdata)(bct_entity_t, bct_fastdata_t *s);
    int (*entity_thing)(bct_entity_t, bct_thing_t *s);
    int (*entity_window)(bct_entity_t, bct_window_t *s);
    bct_data_t (*entity_allocate_wire)(bct_entity_t, unsigned nb, unsigned l, const unsigned char *dp); /* any thread */
    bct_data_t (*entity_allocate_host)(bct_entity_t, unsigned nb, unsigned long long ts, float u, float l, float r, unsigned t, unsigned dl, unsigned char **dp, unsigned vl, float **vp); /* any thread */
    unsigned long long (*entity_time)(bct_entity_t); /* any thread */
    unsigned long long (*entity_time_btoh)(bct_entity_t, unsigned long long); /* any thread */
    unsigned long long (*entity_time_htob)(bct_entity_t, unsigned long long); /* any thread */
    void (*entity_lock)(bct_entity_t); /* any thread except fast */
    void (*entity_unlock)(bct_entity_t); /* any thread except fast */
    void (*entity_kill)(bct_entity_t);
    int (*entity_fastcall)(bct_entity_t, int (*function)(bct_entity_t, void *, void *), void *arg1, void *arg2); /* any thread */
    void *(*entity_alloc)(bct_entity_t, unsigned nb,unsigned size, void (**dealloc)(void *, void *), void **dealloc_arg); /* any thread */
    void (*entity_log)(bct_entity_t, bct_data_t); /* any thread */
    int (*entity_clocksource)(bct_entity_t, bct_data_t, unsigned, unsigned long, bct_clocksource_t *);
    int (*entity_clockdomain)(bct_entity_t, bct_clockdomain_t *);
    int (*entity_killed)(bct_entity_t);
    bct_data_t (*entity_scope)(bct_entity_t);
    bct_data_t (*entity_unique)(bct_entity_t);
    int (*entity_rpcserver)(bct_entity_t, bct_rpcserver_t *rs, const char *id);
    int (*entity_rpcclient)(bct_entity_t, bct_rpcclient_t *rs, const char *id, bct_data_t path, bct_data_t name, bct_data_t val, unsigned long timeout);
    bct_dataqueue_t (*entity_dataqueue)(bct_entity_t, unsigned);
    void (*entity_exit)(bct_entity_t);
    void (*entity_dump)(bct_entity_t);
	void *(*entity_winctx)(bct_entity_t);
	void (*entity_winch)(bct_entity_t, const char *);
	bool (*entity_is_fast)(bct_entity_t);
	bct_entity_t (*entity_new)(bct_entity_t,bool,const char *,const char *);
	void (*entity_incref)(bct_entity_t);
	void (*entity_decref)(bct_entity_t);
    int (*entity_rpcasync)(bct_entity_t, const char *id, bct_data_t path, bct_data_t name, bct_data_t val, unsigned long timeout);
};

#define bct_entity_server(bc,n,s)               ((*(bc))->entity_server((bc),(n),(s)))
#define bct_entity_client(bc,n,c,f)             ((*(bc))->entity_client((bc),(n),(c),(f)))
#define bct_entity_index(bc,n,i)                ((*(bc))->entity_index((bc),(n),(i)))
#define bct_entity_fastdata(bc,f)               ((*(bc))->entity_fastdata((bc),(f)))
#define bct_entity_thing(bc,i)                  ((*(bc))->entity_thing((bc),(i)))
#define bct_entity_window(bc,i)                 ((*(bc))->entity_window((bc),(i)))
#define bct_entity_rpcserver(bc,r,id)           ((*(bc))->entity_rpcserver((bc),(r),(id)))
#define bct_entity_rpcclient(bc,r,id,p,n,v,t)   ((*(bc))->entity_rpcclient((bc),(r),(id),(p),(n),(v),(t)))
#define bct_entity_rpcasync(bc,id,p,n,v,t)      ((*(bc))->entity_rpcasync((bc),(id),(p),(n),(v),(t)))
#define bct_entity_time(bc)                     ((*(bc))->entity_time((bc)))
#define bct_entity_allocate_host(bc,nb,ts,u,l,r,t,dl,dp,vl,vp)  ((*(bc))->entity_allocate_host((bc),(nb),(ts),(u),(l),(r),(t),(dl),(dp),(vl),(vp)))
#define bct_entity_allocate_wire(bc,nb,l,d)     ((*(bc))->entity_allocate_wire((bc),(nb),(l),(d)))
#define bct_entity_lock(bc)                     ((*(bc))->entity_lock((bc)))
#define bct_entity_unlock(bc)                   ((*(bc))->entity_unlock((bc)))
#define bct_entity_kill(bc)                     ((*(bc))->entity_kill((bc)))
#define bct_entity_fastcall(bc,f,a1,a2)         ((*(bc))->entity_fastcall((bc),(f),(a1),(a2)))
#define bct_entity_alloc(bc,nb,s,d,da)          ((*(bc))->entity_alloc((bc),(nb),(s),(d),(da)))
#define bct_entity_free(bc,p)                   ((*(bc))->entity_free((bc),(p)))
#define bct_entity_log(bc,m)                    ((*(bc))->entity_log((bc),(m)))
#define bct_entity_clocksource(bc,n,bs,sr,s)    ((*(bc))->entity_clocksource((bc),(n),(bs),(sr),(s)))
#define bct_entity_clockdomain(bc,d)            ((*(bc))->entity_clockdomain((bc),(d)))
#define bct_entity_dataqueue(bc,n)              ((*(bc))->entity_dataqueue((bc),(n)))
#define bct_entity_killed(bc)                   ((*(bc))->entity_killed((bc)))
#define bct_entity_scope(bc)                    ((*(bc))->entity_scope((bc)))
#define bct_entity_unique(bc)                   ((*(bc))->entity_unique((bc)))
#define bct_entity_exit(bc)                     ((*(bc))->entity_exit((bc)))
#define bct_entity_dump(bc)                     ((*(bc))->entity_dump((bc)))
#define bct_entity_incref(bc)                   ((*(bc))->entity_incref((bc)))
#define bct_entity_decref(bc)                   ((*(bc))->entity_decref((bc)))
#define bct_entity_winctx(bc)                   ((*(bc))->entity_winctx((bc)))
#define bct_entity_winch(bc,m)                  ((*(bc))->entity_winch((bc),(m)))
#define bct_entity_is_fast(bc)                  ((*(bc))->entity_is_fast((bc)))
#define bct_entity_new(bc,gui,u,t)              ((*(bc))->entity_new((bc),(gui),(u),(t)))

/**
 * Host side RPC client operations
 */

struct bct_rpcclient_host_ops_s
{
    void (*rpcclient_close)(bct_rpcclient_host_ops_t **);
};

#define bct_rpcclient_host_close(bc)           ((*((bc)->host_ops))->rpcclient_close)((bc)->host_ops)

/**
 * Plug side RPC client operations
 */

struct bct_rpcclient_plug_ops_s
{
    void (*rpcclient_attached)(bct_rpcclient_t *);
    void (*rpcclient_complete)(bct_rpcclient_t *, bct_entity_t, int, bct_data_t );
};

#define bct_rpcclient_plug_attached(bc)        ((bc)->plug_ops->rpcclient_attached)((bc))
#define bct_rpcclient_plug_complete(bc,e,s,v)  ((bc)->plug_ops->rpcclient_complete)((bc),(e),(s),(v))

struct bct_rpcclient_s
{
    bct_rpcclient_host_ops_t **host_ops;
    bct_rpcclient_plug_ops_t *plug_ops;
};


/**
 * Host side RPC server operations
 */

struct bct_rpcserver_host_ops_s
{
    void (*rpcserver_close)(bct_rpcserver_host_ops_t **);
    void (*rpcserver_complete)(bct_rpcserver_host_ops_t **, void *, int, bct_data_t );
};

#define bct_rpcserver_host_close(bc)            ((*((bc)->host_ops))->rpcserver_close)((bc)->host_ops)
#define bct_rpcserver_host_complete(bc,c,s,v)   ((*((bc)->host_ops))->rpcserver_complete)((bc)->host_ops,(c),(s),(v))

/**
 * Plug side RPC server operations
 */

struct bct_rpcserver_plug_ops_s
{
    void (*rpcserver_attached)(bct_rpcserver_t *);
    void (*rpcserver_invoke)(bct_rpcserver_t *, bct_entity_t, void *, bct_data_t, bct_data_t, bct_data_t );
    void (*rpcserver_closed)(bct_rpcserver_t *, bct_entity_t);
};

#define bct_rpcserver_plug_attached(bc)         ((bc)->plug_ops->rpcserver_attached)((bc))
#define bct_rpcserver_plug_closed(bc,e)         ((bc)->plug_ops->rpcserver_closed)((bc),(e))
#define bct_rpcserver_plug_invoke(bc,e,c,p,n,v) ((bc)->plug_ops->rpcserver_invoke)((bc),(e),(c),(p),(n),(v))

struct bct_rpcserver_s
{
    bct_rpcserver_host_ops_t **host_ops;
    bct_rpcserver_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

/**
 * Host side index operations
 */
struct bct_index_host_ops_s
{
    void (*index_close)(bct_index_host_ops_t **);
    bct_data_t (*index_name)(bct_index_host_ops_t **,int);
    bct_data_t (*index_member_name)(bct_index_host_ops_t **, int,int);
    int (*index_member_count)(bct_index_host_ops_t **);
    unsigned short (*index_member_cookie)(bct_index_host_ops_t **, int);
    void (*index_member_died)(bct_index_host_ops_t **, bct_data_t, unsigned short);
};

/**
 * Plugin side index callbacks
 */
struct bct_index_plug_ops_s
{
    void (*index_attached)(bct_index_t *);
    void (*index_opened)(bct_index_t *, bct_entity_t);
    void (*index_closed)(bct_index_t *, bct_entity_t);
    void (*index_changed)(bct_index_t *, bct_entity_t);
};

#define bct_index_host_close(bc)            ((*((bc)->host_ops))->index_close)((bc)->host_ops)
#define bct_index_host_name(bc,fq)          ((*((bc)->host_ops))->index_name)((bc)->host_ops,(fq))
#define bct_index_host_member_count(bc)     ((*((bc)->host_ops))->index_member_count)((bc)->host_ops)
#define bct_index_host_member_name(bc,i,fq) ((*((bc)->host_ops))->index_member_name)((bc)->host_ops,(i),(fq))
#define bct_index_host_member_cookie(bc,i)  ((*((bc)->host_ops))->index_member_cookie)((bc)->host_ops,(i))
#define bct_index_host_member_died(bc,n,c)  ((*((bc)->host_ops))->index_member_died)((bc)->host_ops,(n),(c))

#define bct_index_plug_attached(bc)         ((bc)->plug_ops->index_attached)((bc))
#define bct_index_plug_opened(bc,c)         ((bc)->plug_ops->index_opened)((bc),(c))
#define bct_index_plug_closed(bc,c)         ((bc)->plug_ops->index_closed)((bc),(c))
#define bct_index_plug_changed(bc,c)        ((bc)->plug_ops->index_changed)((bc),(c))

/**
 * An index provides name lookup services for servers.
 */
struct bct_index_s
{
    bct_index_host_ops_t **host_ops;
    bct_index_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

struct bct_server_host_ops_s
{
    void (*server_host_close)(bct_server_host_ops_t **);
    bct_data_t (*server_host_servername)(bct_server_host_ops_t **,int);
    bct_data_t (*server_host_path)(bct_server_host_ops_t **);
    int (*server_host_child_add)(bct_server_host_ops_t **, unsigned char, bct_server_t *);
    int (*server_host_shutdown)(bct_server_host_ops_t **);
    void (*server_host_changed)(bct_server_host_ops_t **, bct_data_t);
    int (*server_host_advertise)(bct_server_host_ops_t **, const char * );
    int (*server_host_unadvertise)(bct_server_host_ops_t **, const char * );
    void (*server_host_cancel)(bct_server_host_ops_t **);
    void (*server_host_setclock)(bct_server_host_ops_t **, bct_clocksink_t *);
    void (*server_host_set_source)(bct_server_host_ops_t **, bct_fastdata_t *);
    void (*server_host_setflags)(bct_server_host_ops_t **, unsigned);
};

struct bct_server_plug_ops_s
{
    bct_data_t (*server_plug_attached)(bct_server_t *, unsigned *);
    void (*server_plug_opened)(bct_server_t *, bct_entity_t);
    void (*server_plug_closed)(bct_server_t *, bct_entity_t);
};

struct bct_server_s
{
    bct_server_host_ops_t **host_ops;
    bct_server_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

#define bct_server_host_close(bc)               ((*((bc)->host_ops))->server_host_close)((bc)->host_ops)
#define bct_server_host_servername(bc,fq)       ((*((bc)->host_ops))->server_host_servername)((bc)->host_ops,(fq))
#define bct_server_host_path(bc)                ((*((bc)->host_ops))->server_host_path)((bc)->host_ops)
#define bct_server_host_child_add(bc,n,c)       ((*((bc)->host_ops))->server_host_child_add)((bc)->host_ops,(n),(c))
#define bct_server_host_shutdown(bc)            ((*((bc)->host_ops))->server_host_shutdown)((bc)->host_ops)
#define bct_server_host_changed(bc,d)           ((*((bc)->host_ops))->server_host_changed)((bc)->host_ops,(d))
#define bct_server_host_advertise(bc,i)         ((*((bc)->host_ops))->server_host_advertise)((bc)->host_ops,(i))
#define bct_server_host_unadvertise(bc,i)       ((*((bc)->host_ops))->server_host_unadvertise)((bc)->host_ops,(i))
#define bct_server_host_cancel(bc)              ((*((bc)->host_ops))->server_host_cancel)((bc)->host_ops)
#define bct_server_host_setclock(bc,c)          ((*((bc)->host_ops))->server_host_setclock)((bc)->host_ops,(c))
#define bct_server_host_set_source(bc,d)        ((*((bc)->host_ops))->server_host_set_source)((bc)->host_ops,(d))
#define bct_server_host_setflags(bc,f)          ((*((bc)->host_ops))->server_host_setflags)((bc)->host_ops,(f))

#define bct_server_plug_opened(bc,c)        ((bc)->plug_ops->server_plug_opened)((bc),(c))
#define bct_server_plug_closed(bc,c)        ((bc)->plug_ops->server_plug_closed)((bc),(c))
#define bct_server_plug_attached(bc,fp)     ((bc)->plug_ops->server_plug_attached)((bc),(fp))

struct bct_client_host_ops_s
{
    void (*client_host_close)(bct_client_host_ops_t **);
    bct_data_t (*client_host_server_name)(bct_client_host_ops_t **,int);
    bct_data_t (*client_host_path)(bct_client_host_ops_t **);
    int (*client_host_child_add)(bct_client_host_ops_t **, const unsigned char *, unsigned, bct_client_t *);
    int (*client_host_child_remove)(bct_client_host_ops_t **, const unsigned char *, unsigned, bct_client_t *);
    unsigned char (*client_host_child_enum)(bct_client_host_ops_t **, const unsigned char *, unsigned, unsigned char);
    bct_client_t *(*client_host_child_get)(bct_client_host_ops_t **, const unsigned char *, unsigned);
    bct_data_t (*client_host_current_slow)(bct_client_host_ops_t **);
    int (*client_host_clone)(bct_client_host_ops_t **, bct_client_t *);
    bct_data_t (*client_host_child_data)(bct_client_host_ops_t **, const unsigned char *, unsigned, unsigned *);
    int (*client_host_child_exists)(bct_client_host_ops_t **, const unsigned char *, unsigned);
    int (*client_host_cookie)(bct_client_host_ops_t **);
    int (*client_host_shutdown)(bct_client_host_ops_t **);
    int (*client_host_set_downstream)(bct_client_host_ops_t **, bct_clocksink_t *);
    int (*client_host_set_sink)(bct_client_host_ops_t **, bct_fastdata_t *);
    long (*client_host_child_dcrc)(bct_client_host_ops_t **, const unsigned char *, unsigned);
    long (*client_host_child_tcrc)(bct_client_host_ops_t **, const unsigned char *, unsigned);
    long (*client_host_child_ncrc)(bct_client_host_ops_t **, const unsigned char *, unsigned);
    int (*client_host_sync)(bct_client_host_ops_t **);
};

struct bct_client_plug_ops_s
{
    void (*client_plug_attached)(bct_client_t *, bct_data_t);
    void (*client_plug_data)(bct_client_t *, bct_entity_t, bct_data_t);
    void (*client_plug_tree)(bct_client_t *, bct_entity_t);
    void (*client_plug_opened)(bct_client_t *, bct_entity_t);
    void (*client_plug_closed)(bct_client_t *, bct_entity_t);
    void (*client_plug_sync)(bct_client_t *, bct_entity_t);
    void (*client_plug_clock)(bct_client_t *, bct_entity_t);
    void (*client_plug_child)(bct_client_t *, bct_entity_t);
};

struct bct_client_s
{
    bct_client_host_ops_t **host_ops;
    bct_client_plug_ops_t *plug_ops;
    unsigned short plug_flags;
    unsigned short host_flags;
    unsigned short plg_state;
};

#define bct_client_host_close(bc)               ((*((bc)->host_ops))->client_host_close)((bc)->host_ops)
#define bct_client_host_server_name(bc,fq)      ((*((bc)->host_ops))->client_host_server_name)((bc)->host_ops,(fq))
#define bct_client_host_path(bc)                ((*((bc)->host_ops))->client_host_path)((bc)->host_ops)
#define bct_client_host_current_slow(bc)        ((*((bc)->host_ops))->client_host_current_slow)((bc)->host_ops)
#define bct_client_host_child_add(bc,o,l,c)     ((*((bc)->host_ops))->client_host_child_add)((bc)->host_ops,(o),(l),(c))
#define bct_client_host_child_remove(bc,o,l,c)  ((*((bc)->host_ops))->client_host_child_remove)((bc)->host_ops,(o),(l),(c))
#define bct_client_host_child_enum(bc,o,l,ch)   ((*((bc)->host_ops))->client_host_child_enum)((bc)->host_ops,(o),(l),(ch))
#define bct_client_host_child_get(bc,o,l)       ((*((bc)->host_ops))->client_host_child_get)((bc)->host_ops,(o),(l))
#define bct_client_host_update(bc)              ((*((bc)->host_ops))->client_host_update)((bc)->host_ops)
#define bct_client_host_clone(bc,c)             ((*((bc)->host_ops))->client_host_clone)((bc)->host_ops,(c))
#define bct_client_host_child_data(bc,o,ol,fp)  ((*((bc)->host_ops))->client_host_child_data)((bc)->host_ops,(o),(ol),(fp))
#define bct_client_host_child_exists(bc,o,ol)   ((*((bc)->host_ops))->client_host_child_exists)((bc)->host_ops,(o),(ol))
#define bct_client_host_cookie(bc)              ((*((bc)->host_ops))->client_host_cookie)((bc)->host_ops)
#define bct_client_host_shutdown(bc)            ((*((bc)->host_ops))->client_host_shutdown)((bc)->host_ops)
#define bct_client_host_set_downstream(bc,c)    ((*((bc)->host_ops))->client_host_set_downstream)((bc)->host_ops,(c))
#define bct_client_host_set_sink(bc,f)          ((*((bc)->host_ops))->client_host_set_sink)((bc)->host_ops,(f))
#define bct_client_host_child_dcrc(bc,o,ol)     ((*((bc)->host_ops))->client_host_child_dcrc)((bc)->host_ops,(o),(ol))
#define bct_client_host_child_tcrc(bc,o,ol)     ((*((bc)->host_ops))->client_host_child_tcrc)((bc)->host_ops,(o),(ol))
#define bct_client_host_child_ncrc(bc,o,ol)     ((*((bc)->host_ops))->client_host_child_ncrc)((bc)->host_ops,(o),(ol))
#define bct_client_host_sync(bc)                ((*((bc)->host_ops))->client_host_sync)((bc)->host_ops)

#define bct_client_plug_opened(bc,c)            ((bc)->plug_ops->client_plug_opened)((bc),(c))
#define bct_client_plug_closed(bc,c)            ((bc)->plug_ops->client_plug_closed)((bc),(c))
#define bct_client_plug_data(bc,c,d)            ((bc)->plug_ops->client_plug_data)((bc),(c),(d))
#define bct_client_plug_tree(bc,c)              ((bc)->plug_ops->client_plug_tree)((bc),(c))
#define bct_client_plug_sync(bc,c)              ((bc)->plug_ops->client_plug_sync)((bc),(c))
#define bct_client_plug_attached(bc,d)          ((bc)->plug_ops->client_plug_attached)((bc),(d))
#define bct_client_plug_clock(bc,c)             ((bc)->plug_ops->client_plug_clock)((bc),(c))
#define bct_client_plug_child(bc,c)             ((bc)->plug_ops->client_plug_child)((bc),(c))

struct bct_fastdata_host_ops_s
{
    int (*fastdata_close)(bct_fastdata_host_ops_t **);
    int (*fastdata_send)(bct_fastdata_host_ops_t **, bct_data_t, bct_dataqueue_t); /* any thread */
    int (*fastdata_set_upstream)(bct_fastdata_host_ops_t **, bct_fastdata_t *);
    int (*fastdata_enable)(bct_fastdata_host_ops_t **, int e, int *s, int p);
    int (*fastdata_suppress)(bct_fastdata_host_ops_t **, int s, int p); /* fast thread */
    bct_data_t (*fastdata_current)(bct_fastdata_host_ops_t **, int id); /* any thread */
    bct_dataqueue_t (*fastdata_current_queue)(bct_fastdata_host_ops_t **); /* any thread */
    int (*fastdata_send_fast)(bct_fastdata_host_ops_t **, bct_data_t, bct_dataqueue_t); /* fast thread */
    int (*fastdata_subscribe)(bct_fastdata_host_ops_t **); /* fast thread */
};

struct bct_fastdata_plug_ops_s
{
    void (*fastdata_attached)(bct_fastdata_t *);
    int (*fastdata_receive_event)(void *, bct_entity_t, bct_data_t, bct_dataqueue_t); /* fast thread */
    int (*fastdata_receive_data)(void *, bct_entity_t, bct_data_t); /* fast thread */
    void (*fastdata_closed)(bct_fastdata_t *, bct_entity_t);
    void *fastdata_receive_context;
};

struct bct_fastdata_s
{
    bct_fastdata_host_ops_t **host_ops;
    bct_fastdata_plug_ops_t *plug_ops;
    unsigned short plug_flags;
    unsigned short plg_state;
};

#define bct_fastdata_host_close(bc)                 ((*((bc)->host_ops))->fastdata_close)((bc)->host_ops)
#define bct_fastdata_host_send(bc,d,q)              ((*((bc)->host_ops))->fastdata_send)((bc)->host_ops,(d),(q))
#define bct_fastdata_host_send_fast(bc,d,q)         ((*((bc)->host_ops))->fastdata_send_fast)((bc)->host_ops,(d),(q))
#define bct_fastdata_host_set_upstream(bc,f)        ((*((bc)->host_ops))->fastdata_set_upstream)((bc)->host_ops,(f))
#define bct_fastdata_host_enable(bc,e,s,p)          ((*((bc)->host_ops))->fastdata_enable)((bc)->host_ops,(e),(s),(p))
#define bct_fastdata_host_suppress(bc,s,p)          ((*((bc)->host_ops))->fastdata_suppress)((bc)->host_ops,(s),(p))
#define bct_fastdata_host_current(bc,id)            ((*((bc)->host_ops))->fastdata_current)((bc)->host_ops,(id))
#define bct_fastdata_host_current_queue(bc)         ((*((bc)->host_ops))->fastdata_current_queue)((bc)->host_ops)
#define bct_fastdata_host_subscribe(bc)             ((*((bc)->host_ops))->fastdata_subscribe)((bc)->host_ops)

#define bct_fastdata_plug_attached(bc)              ((bc)->plug_ops->fastdata_attached)((bc))
#define bct_fastdata_plug_receive_event(bc,c,d,q)   ((bc)->plug_ops->fastdata_receive_event)((bc)->plug_ops->fastdata_receive_context,(c),(d),(q))
#define bct_fastdata_plug_receive_data(bc,c,d)      ((bc)->plug_ops->fastdata_receive_data)((bc)->plug_ops->fastdata_receive_context,(c),(d))
#define bct_fastdata_plug_closed(bc,c)              ((bc)->plug_ops->fastdata_closed)((bc),(c))

struct bct_window_host_ops_s
{
    void (*window_close)(bct_window_host_ops_t **);
    void (*window_state)(bct_window_host_ops_t **, int);
    void (*window_title)(bct_window_host_ops_t **, const char *);
};

struct bct_window_plug_ops_s
{
    void (*window_attached)(bct_window_t *);
    void (*window_state)(bct_window_t *, bct_entity_t, int);
    void (*window_closed)(bct_window_t *, bct_entity_t);
};

struct bct_window_s
{
    bct_window_host_ops_t **host_ops;
    bct_window_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

#define bct_window_host_close(bc)        ((*((bc)->host_ops))->window_close)((bc)->host_ops)
#define bct_window_host_state(bc,s)      ((*((bc)->host_ops))->window_state)((bc)->host_ops,(s))
#define bct_window_host_title(bc,t)      ((*((bc)->host_ops))->window_title)((bc)->host_ops,(t))

#define bct_window_plug_attached(bc)     ((bc)->plug_ops->window_attached)((bc))
#define bct_window_plug_state(bc,c,s)    ((bc)->plug_ops->window_state)((bc),(c),(s))
#define bct_window_plug_closed(bc,c)     ((bc)->plug_ops->window_closed)((bc),(c))

struct bct_thing_host_ops_s
{
    void (*thing_close)(bct_thing_host_ops_t **);
    void (*thing_trigger_fast)(bct_thing_host_ops_t **); /* any thread */
    void (*thing_trigger_slow)(bct_thing_host_ops_t **); /* any thread */
    void (*thing_queue_fast)(bct_thing_host_ops_t **, bct_data_t, int); /* any thread */
    void (*thing_queue_slow)(bct_thing_host_ops_t **, bct_data_t); /* any thread */
    void (*thing_defer_delete)(bct_thing_host_ops_t **, bool(*)(void*), void *, unsigned long ms); /* any thread */
    int (*thing_timer_fast)(bct_thing_host_ops_t **, unsigned long ms, long us);
    int (*thing_timer_slow)(bct_thing_host_ops_t **, unsigned long ms);
    void (*thing_cancel_timer_fast)(bct_thing_host_ops_t **);
    void (*thing_cancel_timer_slow)(bct_thing_host_ops_t **);
    void (*thing_flush_slow)(bct_thing_host_ops_t **);
};

struct bct_thing_plug_ops_s
{
    void (*thing_attached)(bct_thing_t *);
    void (*thing_triggered_fast)(bct_thing_t *, bct_entity_t);
    void (*thing_triggered_slow)(bct_thing_t *, bct_entity_t);
    void (*thing_dequeue_fast)(bct_thing_t *, bct_entity_t, bct_data_t);
    void (*thing_dequeue_slow)(bct_thing_t *, bct_entity_t, bct_data_t);
    void (*thing_timer_fast)(bct_thing_t *, bct_entity_t);
    void (*thing_timer_slow)(bct_thing_t *, bct_entity_t);
    void (*thing_closed)(bct_thing_t *, bct_entity_t);
};

struct bct_thing_s
{
    bct_thing_host_ops_t **host_ops;
    bct_thing_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

#define bct_thing_host_close(bc)                 ((*((bc)->host_ops))->thing_close)((bc)->host_ops)
#define bct_thing_host_trigger_fast(bc)          ((*((bc)->host_ops))->thing_trigger_fast)((bc)->host_ops)
#define bct_thing_host_trigger_slow(bc)          ((*((bc)->host_ops))->thing_trigger_slow)((bc)->host_ops)
#define bct_thing_host_queue_fast(bc,d,p)        ((*((bc)->host_ops))->thing_queue_fast)((bc)->host_ops,(d),(p))
#define bct_thing_host_queue_slow(bc,d)          ((*((bc)->host_ops))->thing_queue_slow)((bc)->host_ops,(d))
#define bct_thing_host_defer_delete(bc,cb,d,ms)  ((*((bc)->host_ops))->thing_defer_delete)((bc)->host_ops,(cb),(d),(ms))
#define bct_thing_host_timer_fast(bc,ms,us)      ((*((bc)->host_ops))->thing_timer_fast)((bc)->host_ops,(ms),(us))
#define bct_thing_host_timer_slow(bc,ms)         ((*((bc)->host_ops))->thing_timer_slow)((bc)->host_ops,(ms))
#define bct_thing_host_cancel_timer_fast(bc)     ((*((bc)->host_ops))->thing_cancel_timer_fast)((bc)->host_ops)
#define bct_thing_host_cancel_timer_slow(bc)     ((*((bc)->host_ops))->thing_cancel_timer_slow)((bc)->host_ops)
#define bct_thing_host_flush_slow(bc)            ((*((bc)->host_ops))->thing_flush_slow)((bc)->host_ops)

#define bct_thing_plug_attached(bc)              ((bc)->plug_ops->thing_attached)((bc))
#define bct_thing_plug_triggered_fast(bc,c)      ((bc)->plug_ops->thing_triggered_fast)((bc),(c))
#define bct_thing_plug_triggered_slow(bc,c)      ((bc)->plug_ops->thing_triggered_slow)((bc),(c))
#define bct_thing_plug_dequeue_fast(bc,c,d)      ((bc)->plug_ops->thing_dequeue_fast)((bc),(c),(d))
#define bct_thing_plug_dequeue_slow(bc,c,d)      ((bc)->plug_ops->thing_dequeue_slow)((bc),(c),(d))
#define bct_thing_plug_timer_fast(bc,c)          ((bc)->plug_ops->thing_timer_fast)((bc),(c))
#define bct_thing_plug_timer_slow(bc,c)          ((bc)->plug_ops->thing_timer_slow)((bc),(c))
#define bct_thing_plug_closed(bc,c)              ((bc)->plug_ops->thing_closed)((bc),(c))

/**
 *  A clock sink is a recipient of clock ticks from some clock source 
 *  in the system.  It can be notified of changes to the source.  It
 *  also participates in a dependency relationship with other clock
 *  sinks, to ensure correct ordering of clocks down a pipeline of
 *  data.
 */
struct bct_clocksink_s
{
    bct_clocksink_host_ops_t **host_ops;
    bct_clocksink_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

struct bct_clocksink_host_ops_s
{
    int (*clocksink_add_upstream)(bct_clocksink_host_ops_t **, bct_clocksink_t *);
    int (*clocksink_remove_upstream)(bct_clocksink_host_ops_t **, bct_clocksink_t *);
    int (*clocksink_enable)(bct_clocksink_host_ops_t **, int, int);
    void (*clocksink_close)(bct_clocksink_host_ops_t **);
    int (*clocksink_suppress)(bct_clocksink_host_ops_t **, int); /* fast thread */
    void (*clocksink_tick)(bct_clocksink_host_ops_t **); /* fast thread */
    void (*clocksink_tick2)(bct_clocksink_host_ops_t **,void(*)(unsigned long long,unsigned long long,void *),void *); /* fast thread */
    unsigned long long (*clocksink_current_tick)(bct_clocksink_host_ops_t **); /* fast thread */
    unsigned (*clocksink_buffer_size)(bct_clocksink_host_ops_t **); /* fast thread */
    unsigned long (*clocksink_sample_rate)(bct_clocksink_host_ops_t **); /* fast thread */
};

struct bct_clocksink_plug_ops_s
{
    void (*clocksink_ticked)(bct_clocksink_t *, bct_entity_t, unsigned long long, unsigned long long); /* fast thread */
    void (*clocksink_closed)(bct_clocksink_t *, bct_entity_t);
};

#define bct_clocksink_host_add_upstream(bc,s)                   ((*((bc)->host_ops))->clocksink_add_upstream)((bc)->host_ops,(s))
#define bct_clocksink_host_remove_upstream(bc,s)                ((*((bc)->host_ops))->clocksink_remove_upstream)((bc)->host_ops,(s))
#define bct_clocksink_host_enable(bc,f,s)                       ((*((bc)->host_ops))->clocksink_enable)((bc)->host_ops,(f),(s))
#define bct_clocksink_host_suppress(bc,f)                       ((*((bc)->host_ops))->clocksink_suppress)((bc)->host_ops,(f))
#define bct_clocksink_host_close(bc)                            ((*((bc)->host_ops))->clocksink_close)((bc)->host_ops)
#define bct_clocksink_host_tick(bc)                             ((*((bc)->host_ops))->clocksink_tick)((bc)->host_ops)
#define bct_clocksink_host_tick2(bc,cb,ctx)                     ((*((bc)->host_ops))->clocksink_tick2)((bc)->host_ops,(cb),(ctx))
#define bct_clocksink_host_current_tick(bc)                     ((*((bc)->host_ops))->clocksink_current_tick)((bc)->host_ops)
#define bct_clocksink_plug_ticked(bc,x,bf,bt)                   ((bc)->plug_ops->clocksink_ticked)((bc),(x),(bf),(bt))
#define bct_clocksink_plug_closed(bc,x)                         ((bc)->plug_ops->clocksink_closed)((bc),(x))
#define bct_clocksink_host_buffer_size(bc)                      ((*((bc)->host_ops))->clocksink_buffer_size)((bc)->host_ops)
#define bct_clocksink_host_sample_rate(bc)                      ((*((bc)->host_ops))->clocksink_sample_rate)((bc)->host_ops)

/**
 *  A clock source is a source of timing information to which components
 *  in the system may subscribe.
 */
struct bct_clocksource_s
{
    bct_clocksource_host_ops_t **host_ops;
    bct_clocksource_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

struct bct_clocksource_host_ops_s
{
    void (*clocksource_tick)(bct_clocksource_host_ops_t **, unsigned long long); /* fast thread */
    void (*clocksource_set_details)(bct_clocksource_host_ops_t **, unsigned, unsigned long);
    void (*clocksource_close)(bct_clocksource_host_ops_t **);
};

struct bct_clocksource_plug_ops_s
{
    void (*clocksource_closed)(bct_clocksource_t *, bct_entity_t);
};

#define bct_clocksource_host_tick(bc,bt)                     ((*((bc)->host_ops))->clocksource_tick)((bc)->host_ops,(bt))
#define bct_clocksource_host_set_details(bc,bs,sr)           ((*((bc)->host_ops))->clocksource_set_details)((bc)->host_ops,(bs),(sr))
#define bct_clocksource_host_close(bc)                       ((*((bc)->host_ops))->clocksource_close)((bc)->host_ops)
#define bct_clocksource_plug_closed(bc,x)                    ((bc)->plug_ops->clocksource_closed)((bc),(x))

/**
 *  A clock domain is a group of clock sinks.  A clock sink belongs to
 *  one clock domain.  A clock domain has one clock source.  Changing
 *  the source for a domain changes the source for all the sinks in the domain.
 */
struct bct_clockdomain_s
{
    bct_clockdomain_host_ops_t **host_ops;
    bct_clockdomain_plug_ops_t *plug_ops;
    unsigned short plg_state;
};

struct bct_clockdomain_host_ops_s
{
    int (*clockdomain_set_source)(bct_clockdomain_host_ops_t **, bct_data_t);
    bct_data_t (*clockdomain_source_name)(bct_clockdomain_host_ops_t **);
    unsigned (*clockdomain_buffer_size)(bct_clockdomain_host_ops_t **);
    unsigned long (*clockdomain_sample_rate)(bct_clockdomain_host_ops_t **);
    int (*clockdomain_clocksink)(bct_clockdomain_host_ops_t **, bct_clocksink_t *, const char *);
    void (*clockdomain_close)(bct_clockdomain_host_ops_t **);
};

struct bct_clockdomain_plug_ops_s
{
    void (*clockdomain_source_changed)(bct_clockdomain_t *, bct_entity_t);
    void (*clockdomain_closed)(bct_clockdomain_t *, bct_entity_t);
};

#define bct_clockdomain_host_set_source(bc,s)                ((*((bc)->host_ops))->clockdomain_set_source)((bc)->host_ops,(s))
#define bct_clockdomain_host_source_name(bc)                 ((*((bc)->host_ops))->clockdomain_source_name)((bc)->host_ops)
#define bct_clockdomain_host_buffer_size(bc)                 ((*((bc)->host_ops))->clockdomain_buffer_size)((bc)->host_ops)
#define bct_clockdomain_host_sample_rate(bc)                 ((*((bc)->host_ops))->clockdomain_sample_rate)((bc)->host_ops)
#define bct_clockdomain_host_close(bc)                       ((*((bc)->host_ops))->clockdomain_close)((bc)->host_ops)
#define bct_clockdomain_host_clocksink(bc,s,n)               ((*((bc)->host_ops))->clockdomain_clocksink)((bc)->host_ops,(s),(n))
#define bct_clockdomain_plug_source_changed(bc,x)            ((bc)->plug_ops->clockdomain_source_changed)((bc),(x))
#define bct_clockdomain_plug_closed(bc,x)                    ((bc)->plug_ops->clockdomain_closed)((bc),(x))

extern bct_server_t *bct_agent_create(bct_entity_t, const char *path);
extern void bct_agent_delete(bct_entity_t, bct_server_t *);

typedef bct_server_t *(*bct_agent_create_t)(bct_entity_t, const char *);
typedef void (*bct_agent_delete_t)(bct_entity_t, bct_server_t *);

#define PLG_STATUS_OK               /** no error */
#define PLG_STATUS_INTERNAL (-1)    /** internal error */
#define PLG_STATUS_EXISTS   (-2)    /** already exists */
#define PLG_STATUS_NOEXISTS (-3)    /** doesn't exist */
#define PLG_STATUS_STATE    (-4)    /** closed */
#define PLG_STATUS_THREAD   (-5)    /** closed */
#define PLG_STATUS_ADDR     (-6)    /** invalid address */

#ifdef __cplusplus
}
#endif

#endif
