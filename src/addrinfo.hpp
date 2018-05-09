#ifndef _FIBERIO_ADDRINFO_H_
#define _FIBERIO_ADDRINFO_H_

#include <uv.h>
#include <memory>
#include <string>

namespace fiberio {

using addrinfo_ptr =
    std::unique_ptr<struct addrinfo, decltype(&uv_freeaddrinfo)>;

addrinfo_ptr getaddrinfo(const std::string& host, int port);

addrinfo_ptr getaddrinfo(const std::string& node, const std::string& service);

}

#endif
