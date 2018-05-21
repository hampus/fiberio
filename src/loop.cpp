#include "loop.hpp"
#include "utils.hpp"
#include <iostream>
#include <thread>
#include <mutex>

namespace fiberio {

namespace {

constexpr bool DEBUG_LOG = false;

class thread_uv_loop
{
public:
    thread_uv_loop()
        : ready_{false}
    {
        if (DEBUG_LOG) {
            std::cout << "thread created\n";
        }
    }

    ~thread_uv_loop() {
        if (ready_) {
            if (DEBUG_LOG) {
                std::cout << "destroying uv loop\n";
            }
            close_handle((uv_handle_t*) &async_);
            close_handle((uv_handle_t*) &timer_);
            uv_loop_close(&loop_);
        } else {
            if (DEBUG_LOG) {
                std::cout << "thread destroyed without uv loop\n";
            }
        }
    }

    void make_ready() {
        if (!ready_) {
            if (DEBUG_LOG) {
                std::cout << "initializing uv loop\n";
            }
            uv_loop_init(&loop_);
            uv_timer_init(&loop_, &timer_);
            uv_async_init(&loop_, &async_, NULL);
            ready_ = true;
        }
    }

    uv_loop_t* get_loop() {
        make_ready();
        return &loop_;
    }

    uv_timer_t* get_timer() {
        make_ready();
        return &timer_;
    }

    uv_async_t* get_async() {
        make_ready();
        return &async_;
    }
private:
    bool ready_;
    uv_loop_t loop_;
    uv_timer_t timer_;
    uv_async_t async_;
};

thread_local thread_uv_loop thread_loop;

}

uv_loop_t* get_uv_loop()
{
    return thread_loop.get_loop();
}

uv_timer_t* get_scheduler_timer()
{
    return thread_loop.get_timer();
}

uv_async_t* get_scheduler_async()
{
    return thread_loop.get_async();
}

}
