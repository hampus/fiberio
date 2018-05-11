#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>

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

TEST(scheduling, sleeping) {
    fiberio::use_on_this_thread();
    this_fiber::sleep_for(std::chrono::milliseconds(1));
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
    std::string result = futures.at(9).get();
    auto stop = std::chrono::steady_clock::now();
    auto duration = milliseconds_between(start, stop);
    ASSERT_EQ("test", result);
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

    fibers::async(forward_value, promise1.get_future(), &promise2);

    promise1.set_value(33);
    ASSERT_EQ(33, promise2.get_future().get());
}

TEST(scheduling, async_lambda) {
    fiberio::use_on_this_thread();

    int a = 0;
    fibers::async([&a](int b) {
        a = b;
    }, 55).wait();

    ASSERT_EQ(55, a);
}
