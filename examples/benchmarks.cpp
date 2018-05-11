#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <cstdint>
#include <future>
#include <cstring>
#include <vector>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

class time_measure
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;
    using nanoseconds = std::chrono::nanoseconds;
    using seconds = std::chrono::seconds;

    time_measure()
        : start_{ clock::now() }
    {}

    void finish(std::size_t iterations) {
        auto end{ clock::now() };
        auto duration{ end - start_ };
        auto ns{ std::chrono::duration_cast<nanoseconds>(duration).count() };
        double seconds{ ns / 1000000000.0 };
        double milliseconds{ seconds * 1000.0 };
        double iter_per_sec{ iterations / seconds };
        std::cout << "nanoseconds: " << ns << "\n";
        std::cout << "milliseconds: " << milliseconds << "\n";
        std::cout << "seconds: " << seconds << "\n";
        std::cout << "iterations: " << iterations << "\n";
        std::cout << "per iteration: " << ns / iterations << " ns\n";
        std::cout << "iterations per second: " <<
            static_cast<int>(iter_per_sec) << "\n";
    }

private:
    time_point start_;
};

class dummy_lock
{
public:
    void lock() {}
    void unlock() {}
};

void bench_echo_one_byte()
{
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 5502);
    server.listen(50);

    const uint64_t iterations = 500;
    const int num_clients = 100;

    auto server_future = fibers::async([&server]() {
        for (int c = 0; c < num_clients; c++) {
            auto server_client = server.accept();
            fibers::async([](fiberio::socket client) {
                char buf[1];
                for (uint64_t i = 0; i < iterations; i++) {
                    client.read_exactly(buf, sizeof(buf));
                    client.write(buf, sizeof(buf));
                }
            }, std::move(server_client));
        }
    });

    std::vector<fiberio::socket> clients(num_clients);
    for (auto& client : clients) {
        client.connect(server.get_host(), server.get_port());
    }

    char buf[] {'a'};
    char buf2[1];
    time_measure measure;
    for (uint64_t i = 0; i < iterations; i++) {
        for (auto& client : clients) {
            client.write(buf, sizeof(buf));
        }
        for (auto& client : clients) {
            client.read_exactly(buf2, sizeof(buf2));
        }
    }
    measure.finish(iterations * num_clients);

    server_future.get();
    server.close();
}

void bench_fiber_switching()
{
    const uint64_t iterations = 1000'000;
    time_measure measure;

    int shared_int = 0;
    dummy_lock lock;
    fibers::condition_variable_any cond;

    fibers::async([&]() {
        for (uint64_t i = 0; i < iterations; i++) {
            while (shared_int != 1) {
                cond.wait(lock);
            }
            shared_int = 0;
            cond.notify_one();
        }
    });

    for (uint64_t i = 0; i < iterations; i++) {
        while (shared_int != 0) {
            cond.wait(lock);
        }
        shared_int = 1;
        cond.notify_one();
    }
    while (shared_int != 0) {
        cond.wait(lock);
    }
    measure.finish(2*iterations);
}

int main()
{
    std::cout << "bench_echo_one_byte\n";
    std::async(bench_echo_one_byte).get();

    std::cout << "\nbench_fiber_switching\n";
    std::async(bench_fiber_switching).get();
    return 0;
}
