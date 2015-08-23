#include <hls_typedef.h>
#include <hls_core.h>

extern hls_module_t    hls_core_module;
extern hls_module_t    hls_httplive_module;
extern hls_module_t    hls_http_module;

hls_module_t  *hls_modules[] = {
    &hls_core_module,
    &hls_httplive_module,
    &hls_http_module,
     NULL
};
