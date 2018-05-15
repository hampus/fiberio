#include "loop.hpp"
#include "utils.hpp"
#include <iostream>
#include <thread>
#include <mutex>

namespace fiberio {

namespace {

constexpr bool DEBUG_LOG = false;

void print_thread_id()
{
    std::thread::id this_id = std::this_thread::get_id();
    std::cout << "thread " << this_id << ": ";
}

class thread_uv_loop
{
public:
    thread_uv_loop()
        : ready_{false}
    {
        if (DEBUG_LOG) {
            print_thread_id();
            std::cout << "thread created\n";
        }
    }

    ~thread_uv_loop() {
        if (ready_) {
            if (DEBUG_LOG) {
                print_thread_id();
                std::cout << "destroying uv loop\n";
            }
            close_handle(&timer_);
            uv_loop_close(&loop_);
        } else {
            if (DEBUG_LOG) {
                print_thread_id();
                std::cout << "thread destroyed without uv loop\n";
            }
        }
    }

    void make_ready() {
        if (!ready_) {
            if (DEBUG_LOG) {
                print_thread_id();
                std::cout << "initializing uv loop\n";
            }
            uv_loop_init(&loop_);
            uv_timer_init(&loop_, &timer_);
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
private:
    bool ready_;
    uv_loop_t loop_;
    uv_timer_t timer_;
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

}
