#include <hls_typedef.h>
#include <hls_core.h>
#include <hls_event.h>

hls_int_t hls_event_connect_peer(hls_connection_t *c) {
    hls_int_t        rc;
    hls_err_t        err;
    hls_socket_t     s;
    int              rcvbuf;

    s = hls_socket(c->sockaddr->sa_family, SOCK_STREAM, 0);
    if (s == (hls_socket_t)-1) {
        hls_log_error(HLS_LOG_ALERT, c->log, hls_socket_errno, "create socket failed");
        return HLS_ERROR;
    }

    rcvbuf = 1024*1024;
    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const void*)&rcvbuf, sizeof(int)) == -1) {
        hls_log_error(HLS_LOG_ALERT, c->log, hls_socket_errno, "setsockopt(SO_RCVBUF) failed");
        goto failed;
    }

    c->recv = hls_recv;
    c->send = hls_send;

    if (hls_add_conn) {
        if (hls_add_conn(c) == HLS_ERROR) {
            goto failed;
        }
    }

    rc = connect(s, c->sockaddrk, c->socklen);

    if (rc == -1) {
        err = hls_socket_errno;

        if (err != HLS_EINPROGRESS) {
            hls_log_error(HLS_LOG_ERR, c->log, err, "connect() to %s failed", c->name);
            return HLS_ERROR;
        }
    }

    if (hls_add_conn) {
        if (rc == -1) {
            return HLS_AGAIN;
        }

        wev->ready = 1;

        return HLS_OK;
    }

    if (hls_add_event(rev, HLS_READ_EVENT, 0) != HLS_OK) {
        goto failed;
    }

    if (rc == -1) {
        if (hls_add_event(wev, HLS_WRITE_EVENT, 0) != HLS_OK) {
            goto failed;
        }

        return HLS_AGAIN;
    }

    wev->ready = 1;

    return HLS_OK;

failed:

    hls_close_connection(c);

    return HLS_ERROR;
}
