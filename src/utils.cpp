#include "utils.hpp"
#include <boost/fiber/all.hpp>
#include <iostream>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

namespace fiberio {

namespace {

const bool DEBUG_LOG = false;

void on_handle_closed(uv_handle_t* handle)
{
    if (DEBUG_LOG) std::cout << "handle was closed\n";
    void* data = uv_handle_get_data(handle);
    fibers::promise<void>* promise = static_cast<fibers::promise<void>*>(data);
    promise->set_value();
}

}

void close_handle(uv_handle_t* handle)
{
    fibers::promise<void> promise;

    if (DEBUG_LOG) std::cout << "closing handle\n";
    uv_handle_set_data(handle, &promise);
    uv_close(handle, on_handle_closed);

    promise.get_future().get();
}

void check_uv_status(int status) {
    if (status < 0) {
        if (DEBUG_LOG) std::cout << "check_uv_status detected an error: " <<
            uv_err_name(status) << "\n";
        if (status == UV_ENOTCONN || status == UV_EBADF) {
            throw socket_closed_error{};
        } else if (status == UV_EAI_ADDRFAMILY) {
            throw address_family_not_supported_error{};
        } else {
            throw uv_error{status};
        }
    }
}

}
