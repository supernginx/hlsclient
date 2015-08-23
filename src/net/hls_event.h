#ifndef __NET_HLS_EVENT_H
#define __NET_HLS_EVENT_H

#include <hls_define.h>
#include <hls_typedef.h>

#define hls_add_event    hls_event_actions.add
#define hls_del_event    hls_event_actions.del
#define hls_add_conn     hls_event_actions.add_conn
#define hls_del_conn     hls_event_actions.del_conn

#define hls_add_timer    hls_event_add_timer
#define hls_del_timer    hls_event_del_timer

extern  hls_os_io_t      hls_io;

#define hls_recv         hls_io.recv
#define hls_send         hls_io.send

#define HLS_EVENT_MODULE  0x544E5645   /* evmt */

struct hls_event_t {
    void        *data;
    
    unsigned     active:1;
    unsigned     disabled:1
    unsigned     ready:1;
    unsigned     timedout:1;
    unsigned     timer_set:1;
    unsigned     error:1;
    unsigned     write:1;
    unsigned     locked:1;

    hls_event_handler_pt    handler;

    hls_log_t   *log;

    struct sockaddr *sockaddr;
    socklen_t        socklen;

    hls_uint_t       index;

    hls_event_t     *next;
    hls_event_t    **prev
};

typedef struct {
    hls_int_t    (*add)(hls_event_t *ev, int event, uint32_t flags);
    hls_int_t    (*del)(hls_event_t *ev, int event, uint32_t flags);

    hls_int_t    (*add_conn)(hls_connection_t *c);
    hls_int_t    (*del_conn)(hls_connection_t *c, uint32_t flags);

    hls_int_t    (*process_events)(hls_cycle_t *cycle, hls_msec_t timer, uint32_t flags);

    hls_int_t    (*init)(hls_cycle_t *cycle, hls_msec_t timer);
} hls_event_actions_t;

extern hls_event_actions_t    hls_event_actions;
//extern hls_mutex_t            hls_connection_mutex;

typedef struct {
    hls_str_t    *name;
    
    hls_event_actions_t  actions;
} hls_event_module_t;

void hls_process_events_and_timers(hls_cycle_t *cycle);
hls_int_t  hls_handle_read_event(hls_event_t *rev, uint32_t flags);
hls_int_t  hls_handle_write_event(hls_event_t *wev, uint32_t flags);

hls_int_t  hls_event_connect_peer(hls_connection_t *c);

#include <hls_event_timer.h>

#endif /* __NET_HLS_EVENT_H */
