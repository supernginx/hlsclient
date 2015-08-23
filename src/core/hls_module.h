#ifndef __CORE_HLS_MODULE_H
#define __CORE_HLS_MODULE_H

#define HLS_CORE_MODULE    0x45524F43

typedef struct {
    hls_str_t    name;
} hls_core_module_t;

struct hls_module_t {
    hls_uint_t    ctx_index;
    hls_uint_t    index;

    hls_uint_t    version;

    void         *ctx;
    hls_uint_t    type;

    hls_int_t     (*init_master)(hls_log_t *log);
    hls_int_t     (*init_module)(hls_cycle_t *cycle);
    hls_int_t     (*init_thread)(hls_cycle_t *cycle);
    void          (*exit_thread)(hls_cycle_t *cycle);
    void          (*exit_master)(hls_cycle_t *cycle);
};

#endif /* __CORE_HLS_MODULE_H */
