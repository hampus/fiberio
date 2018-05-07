#ifndef _FIBERIO_TCPSOCKET_H_
#define _FIBERIO_TCPSOCKET_H_

#include <memory>
#include <string>
#include <fiberio/addrinfo.hpp>

namespace fiberio {

class tcpsocket
{
public:
    static std::unique_ptr<tcpsocket> create();

    tcpsocket() {};
    virtual ~tcpsocket() {};

    virtual void bind(const addrinfo_ptr& addr) = 0;

    virtual void listen(int backlog) = 0;

    virtual std::unique_ptr<tcpsocket> accept() = 0;

    virtual std::string read() = 0;

    virtual void write(const std::string& data) = 0;

    virtual void close() = 0;

    // Non-copyable and non-movable
    tcpsocket(const tcpsocket&) = delete;
    tcpsocket& operator=(const tcpsocket&) = delete;
    tcpsocket(tcpsocket&&) = delete;
    tcpsocket& operator=(tcpsocket&&) = delete;
};

}

#endif
