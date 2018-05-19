#ifndef _FIBERIO_SRC_UTILS_H_
#define _FIBERIO_SRC_UTILS_H_

#include <stdexcept>
#include <uv.h>
#include <fiberio/exceptions.hpp>
#include <boost/fiber/all.hpp>

namespace fiberio {

class uv_error : public std::runtime_error
{
public:
    uv_error(int status)
        : runtime_error{ std::string{uv_err_name(status)} + ": "
            + uv_strerror(status)}, status_{status} {}

    int get_status() { return status_; }

private:
    int status_;
};

class dummy_lock
{
public:
    void lock() {}
    void unlock() {}
};

void close_handle(uv_handle_t* handle);

inline void close_handle(uv_tcp_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

inline void close_handle(uv_timer_t* handle) {
    close_handle(reinterpret_cast<uv_handle_t*>(handle));
}

void check_uv_status(int status);

}

#endif
