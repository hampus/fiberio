#ifndef _FIBERIO_SRC_UTILS_H_
#define _FIBERIO_SRC_UTILS_H_

#include <stdexcept>
#include <uv.h>

namespace fiberio {

void close_handle(uv_handle_t* handle);

inline void close_handle(uv_tcp_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

inline void close_handle(uv_timer_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

inline void check_uv_status(int status) {
    if (status < 0) {
        throw std::runtime_error(uv_err_name(status));
    }
}

class dummy_lock
{
public:
    void lock() {}
    void unlock() {}
};

}

#endif
