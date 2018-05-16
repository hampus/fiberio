#ifndef _FIBERIO_SRC_ADDRINFO_H_
#define _FIBERIO_SRC_ADDRINFO_H_

#include <uv.h>
#include <memory>
#include <string>
#include <cstdint>

namespace fiberio {

using addrinfo_ptr =
    std::unique_ptr<struct addrinfo, decltype(&uv_freeaddrinfo)>;

addrinfo_ptr getaddrinfo(const std::string& host, uint16_t port);

addrinfo_ptr getaddrinfo(const std::string& node, const std::string& service);

}

#endif
