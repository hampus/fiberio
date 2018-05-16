#include <fiberio/fiberio.hpp>
#include "scheduler.hpp"

namespace fibers = boost::fibers;

namespace fiberio {

void use_on_this_thread()
{
    fibers::use_scheduling_algorithm<fiberio::scheduler>();
}

}
