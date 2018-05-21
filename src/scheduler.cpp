#include "scheduler.hpp"
#include "loop.hpp"
#include <algorithm>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

namespace fiberio {

namespace {

const bool DEBUG_LOG = false;

uint64_t milliseconds_until(
    const std::chrono::steady_clock::time_point& abs_time)
{
    using std::chrono::duration_cast;
    auto now = std::chrono::steady_clock::now();
    if (now >= abs_time) return 0;
    return duration_cast<std::chrono::milliseconds>(abs_time - now).count() + 1;
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

    // Make sure the event loop really wakes up (which is sometimes necessary)
    uv_async_send(get_scheduler_async());
}

}

scheduler::scheduler()
    : wake_up_{false}, suspended_{false}
{
    if (DEBUG_LOG) std::cout << "scheduler: creating scheduler\n";
    loop_ = get_uv_loop();
    timer_ = get_scheduler_timer();
    async_ = get_scheduler_async();
}

scheduler::~scheduler()
{
    if (DEBUG_LOG) std::cout << "scheduler: destroying scheduler\n";
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
    if (DEBUG_LOG) {
        if (queue_.empty()) {
            std::cout << "scheduler::has_ready_fibers() -> false\n";
        } else {
            std::cout << "scheduler::has_ready_fibers() -> true\n";
        }
    }
    return !queue_.empty();
}

void scheduler::suspend_until(
    const std::chrono::steady_clock::time_point& abs_time) noexcept
{
    if (DEBUG_LOG) std::cout << "scheduler::suspend_until()\n";

    const bool wake_up_directly{ suspend_enter() };
    if (wake_up_directly) {
        if (DEBUG_LOG) std::cout << "not running event loop\n";
        suspend_exit();
        return;
    }

    bool timer_done{ false };
    bool timer_set{ false };
    if (is_max_time(abs_time)) {
        if (DEBUG_LOG) std::cout << "waiting indefinitely\n";
    } else {
        // We always set the timer to at least 1 ms, since we shouldn't get
        // called at all otherwise and don't need to optimize for that.
        const uint64_t milliseconds{
            std::max<uint64_t>(1, milliseconds_until(abs_time)) };
        if (DEBUG_LOG) std::cout << "setting timer for " <<
            milliseconds << " ms\n";
        timer_set = true;
        uv_handle_set_data((uv_handle_t*) timer_, &timer_done);
        uv_timer_start(timer_, suspend_timer_callback, milliseconds, 0);
    }

    while (true) {
        if (DEBUG_LOG) std::cout << "running event loop\n";
        const int result = uv_run(loop_, UV_RUN_ONCE);
        const bool ready_fibers = !queue_.empty();
        const bool wake_up = should_wake_up();

        if (DEBUG_LOG) {
            std::cout << "uv_run returned " << result << ", ";
            if (timer_done) {
                std::cout << "timer_done: true, ";
            } else {
                std::cout << "timer_done: false, ";
            }
            if (ready_fibers) {
                std::cout << "has_ready_fibers: true, ";
            } else {
                std::cout << "has_ready_fibers: false, ";
            }
            if (wake_up) {
                std::cout << "should_wake_up: true\n";
            } else {
                std::cout << "should_wake_up: false\n";
            }
        }

        if (ready_fibers || timer_done || wake_up) {
            if (DEBUG_LOG) std::cout << "exiting from suspend_until()\n";
            break;
        }
    }

    if (timer_set && !timer_done) {
        if (DEBUG_LOG) std::cout << "stopping timer\n";
        uv_timer_stop(timer_);
    }

    suspend_exit();
}

bool scheduler::suspend_enter()
{
    std::lock_guard<std::mutex> lock{ mutex_ };
    suspended_ = true;
    return wake_up_;
}

bool scheduler::should_wake_up()
{
    std::lock_guard<std::mutex> lock{ mutex_ };
    return wake_up_;
}

void scheduler::suspend_exit()
{
    std::lock_guard<std::mutex> lock{ mutex_ };
    wake_up_ = false;
    suspended_ = false;
}

bool scheduler::update_notify()
{
    std::lock_guard<std::mutex> lock{ mutex_ };
    wake_up_ = true;
    return suspended_;
}

void scheduler::notify() noexcept
{
    if (DEBUG_LOG) std::cout << "scheduler::notify()\n";
    bool suspended = update_notify();
    if (suspended) {
        if (DEBUG_LOG) std::cout << "scheduler uv_async_send()\n";
        uv_async_send(async_);
    } else {
        if (DEBUG_LOG) std::cout << "scheduler not suspended\n";
    }
}

}
