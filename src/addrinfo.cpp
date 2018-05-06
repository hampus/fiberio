#include <fiberio/addrinfo.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>

namespace fibers = boost::fibers;

namespace fiberio {

namespace {

void addrinfo_callback(uv_getaddrinfo_t* req, int status, struct addrinfo* res)
{
    void* data = uv_req_get_data((uv_req_t*) req);
    fibers::promise<struct addrinfo*>* promise =
        static_cast<fibers::promise<struct addrinfo*>*>(data);
    promise->set_value(res);
}

}

addrinfo_ptr getaddrinfo(const std::string& node, const std::string& service)
{
    fibers::promise<struct addrinfo*> promise;

    uv_getaddrinfo_t req;
    uv_req_set_data((uv_req_t*) &req, &promise);

    uv_getaddrinfo(uv_default_loop(), &req, addrinfo_callback,
        node.c_str(), service.c_str(), 0);

    struct addrinfo* res = promise.get_future().get();

    return addrinfo_ptr(res, uv_freeaddrinfo);
}

}
