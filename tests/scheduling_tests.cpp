#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <future>
#include <thread>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

template<class T>
uint64_t milliseconds_between(T start, T end)
{
    using std::chrono::duration_cast;
    return duration_cast<std::chrono::milliseconds>(end - start).count();
}

TEST(scheduling, use_on_this_thread) {
    fiberio::use_on_this_thread();
}

TEST(scheduling, sleeping_zero) {
    fiberio::use_on_this_thread();
    this_fiber::sleep_for(std::chrono::milliseconds{0});
}

TEST(scheduling, sleeping_one_microsecond) {
    fiberio::use_on_this_thread();
    this_fiber::sleep_for(std::chrono::microseconds{1});
}

TEST(scheduling, sleeping_one_millisecond) {
    fiberio::use_on_this_thread();
    this_fiber::sleep_for(std::chrono::milliseconds{1});
}

TEST(scheduling, sleeping_one_hundred_milliseconds) {
    fiberio::use_on_this_thread();
    this_fiber::sleep_for(std::chrono::milliseconds{100});
}

TEST(scheduling, sleeping_one_millisecond_many_times) {
    fiberio::use_on_this_thread();
    for (int i = 0; i < 1000; i++) {
        this_fiber::sleep_for(std::chrono::milliseconds{1});
    }
}

std::string sleep_and_return(int millis, std::string text)
{
    this_fiber::sleep_for(std::chrono::milliseconds(millis));
    return text;
}

TEST(scheduling, multiple_fibers) {
    fiberio::use_on_this_thread();
    auto start = std::chrono::steady_clock::now();
    std::vector<fibers::future<std::string>> futures;
    for (int i = 0; i < 10; i++) {
        futures.push_back(fibers::async(sleep_and_return, 5, "test"));
    }
    for (int i = 0; i < 10; i++) {
        std::string result = futures.at(9 - i).get();
        ASSERT_EQ("test", result);
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = milliseconds_between(start, stop);
    ASSERT_GE(duration, 5);
    ASSERT_LE(duration, 25);
}

void forward_value(fibers::future<int> f1, fibers::promise<int>* p2)
{
    int value = f1.get();
    p2->set_value(value);
}

TEST(scheduling, block_on_promises) {
    fiberio::use_on_this_thread();

    fibers::promise<int> promise1;
    fibers::promise<int> promise2;

    auto fiber = fibers::async(forward_value, promise1.get_future(), &promise2);

    promise1.set_value(33);
    ASSERT_EQ(33, promise2.get_future().get());
    fiber.get();
}

TEST(scheduling, async_lambda) {
    fiberio::use_on_this_thread();

    int a = 0;
    fibers::async([&a](int b) {
        a = b;
    }, 55).wait();

    ASSERT_EQ(55, a);
}

TEST(scheduling, multithreaded_promise_resolution) {
    fiberio::use_on_this_thread();

    fibers::promise<int> promise;

    auto thread = std::async(std::launch::async, [&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        promise.set_value(78);
    });

    ASSERT_EQ(78, promise.get_future().get());
    thread.get();
}

TEST(scheduling, multithreaded_promise_while_busy) {
    fiberio::use_on_this_thread();

    fibers::promise<int> promise1;
    fibers::promise<int> promise2;

    auto thread = std::async(std::launch::async, [&promise1, &promise2]() {
        promise2.set_value(73);
        promise1.set_value(72);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    ASSERT_EQ(72, promise1.get_future().get());
    ASSERT_EQ(73, promise2.get_future().get());
    thread.get();
}

TEST(scheduling, abort_sleeping) {
    fiberio::use_on_this_thread();

    fibers::promise<int> promise;
    auto future = promise.get_future();

    auto thread = std::async(std::launch::async, [&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        promise.set_value(350);
    });

    auto fiber = fibers::async([]() {
        this_fiber::sleep_for(std::chrono::milliseconds{400});
    });

    ASSERT_EQ(350, future.get());
    thread.get();
    fiber.get();
}
