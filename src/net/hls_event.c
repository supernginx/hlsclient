#include <hls_typedef.h>
#include <hls_define.h>

#include <hls_event.h>

hls_module_t         hls_select_module;

hls_event_actions_t  hls_event_actions;


static hls_int_t  hls_event_master_init(hls_cycle_t *cycle);
static void       hls_event_master_exit(hls_cycle_t *cycle);
static hls_int_t  hls_event_module_init(hls_cycle_t *cycle);

static hls_str_t  event_core_name = hls_string("event_core");

hls_event_module_t hls_event_core_module_ctx = {
    &event_core_name,
    {NULL, NULL, NULL, NULL, NULL, NULL}
};

hls_module_t hls_event_core_module = {
    HLS_MODULE_V1,
    &hls_event_core_module_ctx,
    HLS_EVENT_MODULE,
    hls_event_master_init,    /* master */
    hls_event_module_init,    /* module */
    NULL,    /* init thread */
    NULL,    /* exit thread */
    hls_event_master_exit     /* exit master */
};

void hls_process_events_and_timers(hls_cycle_t *cycle) {

}

hls_int_t hls_handle_read_event(hls_event_t *rev, uint32_t flags) {
    if (!rev->active && !rev->ready) {
        if (hls_add_event(rev, HLS_READ_EVENT, 0) == HLS_ERROR) {
            return HLS_ERROR;
        }
    }

    return HLS_OK;
}

hls_int_t hls_handle_write_event(hls_event_t *wev, uint32_t flags) {
    if (!wev->active && !wev->ready) {
        if (hls_add_event(wev, HLS_WRITE_EVENT, 0) == HLS_ERROR) {
            return HLS_ERROR;
        }
    }

    return HLS_OK;
}

static hls_int_t  hls_event_master_init(hls_cycle_t *cycle) {
    return HLS_OK;
}

static void       hls_event_master_exit(hls_cycle_t *cycle) {

}

static hls_int_t  hls_event_module_init(hls_cycle_t *cycle) {
    return HLS_OK;
}
