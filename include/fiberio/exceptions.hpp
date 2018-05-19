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

}

#endif
