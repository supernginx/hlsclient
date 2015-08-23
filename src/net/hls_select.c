#include <hls_typedef.h>
#include <hls_define.h>

#include <hls_event.h>

static hls_int_t  hls_select_init(hls_cycle_t *cycle, hls_msec_t timer);
static hls_int_t  hls_select_add_event(hls_event_t *ev, hls_int_t event, 
    uint32_t flags);
static hls_int_t  hls_select_del_event(hls_event_t *ev, hls_int_t event,
    uint32_t flags);
static hls_int_t  hls_select_process_events(hls_cycle_t *cycle, hls_msec_t timer,
    uint32_t flags);

static fd_set        master_read_fd_set;
static fd_set        master_write_fd_set;
static fd_set        work_read_fd_set;
static fd_set        work_write_fd_set;

static hls_int_t     max_fd;
static hls_uint_t    nevents;

static hls_event_t  **event_index;

static hls_str_t     select_name = hls_string("select");

hls_event_module_t   hls_select_module_ctx = {
    &select_name,
    {
        hls_select_add_event,
        hls_select_del_event,
        hls_select_add_event,
        hls_select_del_event,
        hls_select_process_events,
        hls_select_init
    }    
};

hls_module_t hls_select_module = {
    HLS_MODULE_V1,
    &hls_select_module_ctx,
    HLS_EVENT_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static hls_int_t hls_select_init(hls_cycle_t *cycle, hls_msec_t timer) {
    hls_event_t    **index;

    nevents = 0;
    
    FD_ZERO(&master_read_fd_set);
    FD_ZERO(&master_write_fd_set);

    index = hls_alloc(sizeof(hls_event_t *) * 2 * 1000);
    if (index == NULL) {
        return HLS_ERROR;
    }

    event_index = index;

    hls_io = hls_os_io;
    
    hls_event_actions = hls_select_module_ctx.actions;

    max_fd = -1;

    return HLS_OK;
}

static hls_int_t hls_select_add_event(hls_event_t *ev, hls_int_t event, uint32_t flags) {
    hls_connection_t    *c;

    c = ev->data;

    if ((ev->write && event == HLS_READ_EVENT)
        || (!ev->write && event == HLS_WRITE_EVENT)) {

        hls_log_error(HLS_LOG_ALERT, ev->log, 0, "invalid select %s event fd:%d ev:%ld",
            ev->write?"write":"read", c->fd, event);
        return HLS_ERROR;
    }

    if (event == HLS_READ_EVENT) {
        FD_SET(c->fd, &master_read_fd_set);
    } else if (event == HLS_WRITE_EVENT) {
        FD_SET(c->fd, &master_write_fd_set);
    }

    if (max_fd != -1 && max_fd < c->fd) {
        max_fd = c->fd;
    }

    ev->active = 1;
    event_index[nevents] = ev;
    ev->index = nevents;
    nevents++;

    return HLS_OK;
}

static hls_int_t hls_select_del_event(hls_event_t *ev, hls_int_t event, uint32_t flags) {
    hls_connection_t   *c;
    hls_event_t        *e;

    c = ev->data;

    ev->active = 0;

    if (ev->index == HLS_INVALID_INDEX) {
        return HLS_OK;
    }

    if (event == HLS_READ_EVENT) {
        FD_CLR(c->fd, &master_read_fd_set);
    } else if (event == HLS_WRITE_EVENT) {
        FD_CLR(c->fd, &master_write_fd_set);
    }

    if (max_fd == c->fd) {
        max_fd = -1;
    }

    if (ev->index < --nevents) {
        e = event_index[nevents];
        event_index[ev->index] = e;
        e->index = ev->index;
    }

    ev->index = HLS_INVALID_INDEX;

    return HLS_OK;
}

static hls_int_t hls_select_process_events(hls_cycle_t *cycle, hls_msec_t timer, uint32_t flags) {
    int         ready, nready;
    hls_err_t   err;
    hls_uint_t  i, found;
    hls_event_t *ev, **queue;
    struct timeval tv, *tp;
    hls_connection_t   *c;

    if (max_fd == -1) {
        for (i = 0; i < nevents; i++) {
            c = event_index[i]->data;
            if (max_fd < c->fd) {
                max_fd = c->fd;
            }
        }
        hls_log_debug(HLS_LOG_DEBUG, cycle->log, 0, "change max_fd: %d", max_fd);
    }

    if (timer == HLS_TIMER_INFINITE) {
        tp = NULL;
    } else {
        tv.tv_sec = (long)(timer/1000);
        tv.tv_usec = (long)((timer%1000)*1000);
        tp = &tv;
    }

    hls_log_debug(HLS_LOG_DEBUG, cycle->log, 0, "select timer: %lu", timer);

    work_read_fd_set = master_read_fd_set;
    work_write_fd_set = master_write_fd_set;

    ready = select(max_fd+1, &work_read_fd_set, &work_write_fd_set, NULL, tp);

    err = (ready == -1) ? hls_errno : 0;

    hls_time_update();

    if (err) {
        hls_uint_t    level;

        if (err == HLS_EINTR) {
            level = HLS_LOG_INFO;
        } else {
            level = HLS_LOG_ALERT;
        }
    
        hls_log_error(level, cycle->log, err, "select() failed");

        return HLS_ERROR;
    }

    if (ready == 0) {
        if (timer != HLS_TIMER_INFINITE) {
            return HLS_OK;
        }

        hls_log_error(HLS_LOG_ALERT, cycle->log, 0, "select() returned no events without timeout")
        return HLS_ERROR;
    }

    hls_mutex_lock(hls_posted_events_mutex);

    nready = 0;

    for (i = 0; i < nevents; i++) {
        ev = event_index[i];
        c = ev->data;
        found = 0;

        if (ev->write) {
            if (FD_ISSET(c->fd, &work_write_fd_set)) {
                found = 1;
                hls_log_debug(HLS_LOG_DEBUG, cycle->log, 0, "select write %d", c->fd);
            }
        } else {
            if (FD_ISSET(c->fd, &work_read_fd_set)) {
                found = 1;
                hls_log_debug(HLS_LOG_DEBUG, cycle->log, 0, "select read %d", c->fd);
            }
        }

        if (found) {
            ev->ready = 1;

            hls_locked_posted_event(ev, hls_posted_events);

            nready++;
        }
    }

    hls_mutex_unlock(hls_posted_events_mutex);

    if (ready != nready) {
        hls_log_error(HLS_LOG_ALERT, cycle->log, 0, "select ready != events: %d:%d", ready, nready);
        hls_select_repair_fd_sets(cycle);
    }

    return HLS_OK;
}

static void hls_select_repair_fd_sets(hls_cycle_t *cycle) {
    int        n;
    socklen_t  len;
    hls_err_t  err;
    hls_socket_t s;

    for (s = 0; s <= max_fd; s++) {
        if (FD_ISSET(s, &master_read_fd_set) == 0) {
            continue;
        }

        len = sizeof(int);

        if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n) == -1) {
            err = hls_socket_errno;

            hls_log_error(HLS_LOG_ALERT, cycle->log, err, "invalid description #%d in read fd_set", s);
 
            FD_CLR(s, &master_read_fd_set);
        }
    }

    for (s = 0; s <= max_fd; s++) {
        if (FD_ISSET(s, &master_write_fd_set) == 0) {
            continue;
        }

        hls_log_error(HLS_LOG_ALERT, cycle->log, err, "invalid description #%d in write fd_set", s);

        FD_CLR(s, &master_write_fd_set);
    }

    max_fd = -1;
}
