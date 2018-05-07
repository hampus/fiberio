#include <fiberio/detail/utils.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

namespace fiberio {
namespace detail {

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

}
}
