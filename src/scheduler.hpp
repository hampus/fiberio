#ifndef _FIBERIO_DETAIL_SCHEDULER_H_
#define _FIBERIO_DETAIL_SCHEDULER_H_

#include <boost/fiber/all.hpp>
#include <chrono>
#include <deque>
#include <uv.h>

namespace fiberio {

class scheduler : public boost::fibers::algo::algorithm
{
public:
    scheduler();

    scheduler(uv_loop_t* loop);

    ~scheduler();

    void awakened(boost::fibers::context* fiber) noexcept;

    boost::fibers::context* pick_next() noexcept;

    bool has_ready_fibers() const noexcept;

    void suspend_until(
        std::chrono::steady_clock::time_point const& abs_time) noexcept;

    void notify() noexcept;

    // Non-copyable and non-movable
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;
    scheduler(scheduler&&) = delete;
    scheduler& operator=(scheduler&&) = delete;

private:
    std::deque<boost::fibers::context*> queue_;
    uv_loop_t* loop_;
    uv_timer_t timer_;
};

}

#endif
