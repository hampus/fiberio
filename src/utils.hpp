#ifndef _FIBERIO_SRC_H_
#define _FIBERIO_SRC_H_

#include <uv.h>

namespace fiberio {

void close_handle(uv_handle_t* handle);

inline void close_handle(uv_tcp_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

inline void close_handle(uv_timer_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

}

#endif
