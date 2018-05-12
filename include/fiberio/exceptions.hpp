#ifndef _FIBERIO_EXCEPTIONS_H_
#define _FIBERIO_EXCEPTIONS_H_

#include <stdexcept>

namespace fiberio {

//! This is thrown when something fails because the connection was closed
class socket_closed_error : public std::runtime_error
{
public:
    socket_closed_error() : runtime_error("stream closed") {}
};

}

#endif
