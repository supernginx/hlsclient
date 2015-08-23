#ifndef __CORE_HLS_TYPEDEF_H
#define __CORE_HLS_TYPEDEF_H

#if (_WIN32 || WIN32)

#include <os/win32/hls_win32.h>

#else 

#include <os/unix/hls_unix.h>

#endif

typedef intptr_t    hls_int_t;
typedef uintptr_t   hls_uint_t;

#endif /* __CORE_HLS_TYPEDEF_H */
