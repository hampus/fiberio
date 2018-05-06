#include "fiberio/detail/scheduler.hpp"

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

namespace fiberio {
namespace detail {

namespace {

bool DEBUG_LOG = false;

uint64_t milliseconds_until(
    const std::chrono::steady_clock::time_point& abs_time)
{
    using std::chrono::duration_cast;
    auto now = std::chrono::steady_clock::now();
    if (now >= abs_time) return 0;
    return duration_cast<std::chrono::milliseconds>(abs_time - now).count();
}

bool is_max_time(const std::chrono::steady_clock::time_point& abs_time)
{
    return abs_time == std::chrono::steady_clock::time_point::max();
}

void suspend_timer_callback(uv_timer_t* handle)
{
    if (DEBUG_LOG) std::cout << "suspend_timer_callback\n";
    void* data = uv_handle_get_data((uv_handle_t*) handle);
    bool* timer_done = static_cast<bool*>(data);
    *timer_done = true;
}

}

scheduler::scheduler()
    : scheduler(uv_default_loop())
{
}

scheduler::scheduler(uv_loop_t* loop)
{
    if (DEBUG_LOG) std::cout << "creating scheduler\n";
    loop_ = loop;
    uv_timer_init(loop_, &timer_);
}

scheduler::~scheduler()
{
    // Note: we don't close timer_, since it's usually too late at this point.
    if (DEBUG_LOG) std::cout << "closing the event loop\n";
    uv_loop_close(loop_);
}

void scheduler::awakened(fibers::context* fiber) noexcept
{
    if (DEBUG_LOG) std::cout << "scheduler::awakened(" <<
        fiber->get_id() << ")\n";
    queue_.push_back(fiber);
}

fibers::context* scheduler::pick_next() noexcept
{
    if (queue_.empty()) {
        if (DEBUG_LOG) std::cout << "scheduler::pick_next() -> (no fiber)\n";
        return nullptr;
    } else {
        fibers::context* fiber = queue_.front();
        queue_.pop_front();
        if (DEBUG_LOG) std::cout << "scheduler::pick_next() -> " <<
            fiber->get_id() << "\n";
        return fiber;
    }
}

bool scheduler::has_ready_fibers() const noexcept
{
    if (queue_.empty()) {
        std::cout << "scheduler::has_ready_fibers() -> false\n";
    } else {
        std::cout << "scheduler::has_ready_fibers() -> true\n";
    }
    return !queue_.empty();
}

void scheduler::suspend_until(
    const std::chrono::steady_clock::time_point& abs_time) noexcept
{
    if (DEBUG_LOG) std::cout << "scheduler::suspend_until()\n";
    bool timer_done = false;
    bool timer_set = false;
    if (is_max_time(abs_time)) {
        if (DEBUG_LOG) std::cout << "waiting indefinitely\n";
    } else {
        uint64_t milliseconds = milliseconds_until(abs_time);
        if (milliseconds > 0) {
            milliseconds += 1; // make sure we wait at least long enough
            if (DEBUG_LOG) std::cout << "setting timer for " <<
                milliseconds << " ms\n";
            timer_set = true;
            uv_handle_set_data((uv_handle_t*) &timer_, &timer_done);
            uv_timer_start(&timer_, suspend_timer_callback, milliseconds, 0);
        } else {
            // No need to set a timer. We're already at abs_time (or past).
            timer_done = true;
        }
    }
    while (true) {
        if (DEBUG_LOG) std::cout << "running event loop\n";
        const int result = uv_run(loop_, UV_RUN_ONCE);
        const bool ready_fibers = !queue_.empty();

        if (DEBUG_LOG) {
            std::cout << "uv_run returned " << result << ", ";
            if (timer_done) {
                std::cout << "timer_done: true, ";
            } else {
                std::cout << "timer_done: false, ";
            }
            if (ready_fibers) {
                std::cout << "has_ready_fibers: true\n";
            } else {
                std::cout << "has_ready_fibers: false\n";
            }
        }

        if (ready_fibers || timer_done) {
            if (timer_set && !timer_done) {
                if (DEBUG_LOG) std::cout << "stopping timer\n";
                uv_timer_stop(&timer_);
            }
            if (DEBUG_LOG) std::cout << "exiting from suspend_until()\n";
            break;
        }
    }
}

void scheduler::notify() noexcept
{
    std::cout << "scheduler::notify() (NOT IMPLEMENTED)\n";
    // TODO: not yet supported
}

}
}
