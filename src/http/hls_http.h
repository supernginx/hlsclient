#ifndef __HTTP_HLS_HTTP_H
#define __HTTP_HLS_HTTP_H

#include <hls_typedef.h>
#include <hls_define.h>
#include <hls_core.h>

typedef struct {
    hls_str_t     request;
    hls_str_t     host;
    hls_str_t     accept;
    hls_str_t     pragma;
    hls_str_t     cache_control;
    hls_str_t     referer;
    hls_str_t     user_agent;
    hls_str_t     range;

    hls_str_t     user;
    hls_str_t     password;

} hls_http_header_request_t;

typedef struct {
    hls_buf_t    *buf;
    off_t         rest;
} hls_http_body_request_t;

typedef struct {
    hls_str_t     http_ok;
    hls_str_t     date;
    hls_str_t     server;
    hls_str_t     content_type;
    hls_str_t     last_modified;
    hls_str_t     etag;
    hls_str_t     content_length;
    hls_str_t     content_range;
} hls_http_header_response_t;

typedef hls_int_t (*hls_http_handler_pt)(hls_http_request_t *r);
typedef void (*hls_http_event_handler_pt)(hls_http_request_t *r);

struct hls_http_request_t {
    uint32_t                     signature;

    hls_log_t                   *log;
    
    hls_connection_t            *connection;

    hls_http_event_handler_pt    read_event_handler;
    hls_http_event_handler_pt    write_event_handler;

    hls_buf_t                   *header_in;

    hls_http_header_request_t    headers_in;
    hls_http_header_response_t   header_out;

    hls_http_request_body_t     *request_body;

    time_t                       lingering_time;
    time_t                       start_sec;
    time_t                       start_msec;

    hls_uint_t                   method;
    hls_uint_t                   http_version;

    hls_str_t                    request_line;
    hls_str_t                    uri;
    hls_str_t                    args;
    hls_str_t                    exten;
    hls_str_t                    unparsed_uri;

    hls_str_t                    method_name;
    hls_str_t                    http_protocol;

    hls_str_t                    host;
    int                          port;
    char                         filename[255];
    hls_int_t                    connect_timeout;
    hls_int_t                    rw_timeout;
	hls_int_t                    rc;

    uint8_t                     *uri_start;
    uint8_t                     *uri_end;
    uint8_t                     *uri_ext;
    uint8_t                     *args_start;
    uint8_t                     *request_start;
    uint8_t                     *request_end;
    uint8_t                     *schema_start;
    uint8_t                     *schema_end;
    uint8_t                     *host_start;
    uint8_t                     *host_end;
    uint8_t                     *port_start;
    uint8_t                     *port_end;

    unsigned                     request_sent:1;

    unsigned                     http_minor:16;
    unsigned                     http_major:16;
};

hls_http_request_t * hls_http_create_request(hls_log_t *log);
hls_int_t  hls_http_request(const char *url);
void hls_http_destroy_request(hls_http_request_t *r);

#endif /* __HTTP_HLS_HTTP_H */
