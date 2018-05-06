#include <fiberio/fiberio.hpp>
#include <fiberio/detail/scheduler.hpp>

namespace fibers = boost::fibers;

namespace fiberio {

void use_on_this_thread()
{
    fibers::use_scheduling_algorithm<fiberio::detail::scheduler>();
}

}
