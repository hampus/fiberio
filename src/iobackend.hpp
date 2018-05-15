#ifndef _FIBERIO_SRC_EVENT_BACKEND_H_
#define _FIBERIO_SRC_EVENT_BACKEND_H_

#include "config.hpp"

#if defined USE_EPOLL
#error "epoll backend not implemented"
#elif defined USE_LIBUV
#include "uv/all.hpp"
#else
#error "No I/O backend selected!"
#endif

#endif
