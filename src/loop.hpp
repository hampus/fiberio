#ifndef _FIBERIO_SRC_LOOP_H_
#define _FIBERIO_SRC_LOOP_H_

#include <uv.h>

namespace fiberio {

uv_loop_t* get_uv_loop();

uv_timer_t* get_scheduler_timer();

uv_async_t* get_scheduler_async();

}

#endif
