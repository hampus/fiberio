#ifndef _FIBERIO_EXCEPTIONS_H_
#define _FIBERIO_EXCEPTIONS_H_

#include <stdexcept>

namespace fiberio {

//! This is thrown on I/O errors
class io_error : public std::runtime_error
{
public:
    io_error(const char* msg) : runtime_error{msg} {}
};


//! This is thrown when something fails because the connection was closed
class socket_closed_error : public io_error
{
public:
    socket_closed_error() : io_error{"stream closed"} {}
};

//! Thrown when e.g. trying to use IPv6 and it's not supported on the system
class address_family_not_supported_error : public io_error
{
public:
    address_family_not_supported_error()
        : io_error{"address family not supported"} {}
};


}

#endif
