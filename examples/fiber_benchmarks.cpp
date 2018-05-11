#include "time_measure.hpp"
#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <cstdint>
#include <future>
#include <cstring>
#include <vector>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

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

    const uint64_t num_iterations{ 500 };
    const int num_clients{ 100 };

    auto server_future = fibers::async([&server]() {
        for (int c = 0; c < num_clients; c++) {
            auto server_client = server.accept();
            fibers::async([](fiberio::socket client) {
                char buf[1];
                for (uint64_t i = 0; i < num_iterations; i++) {
                    client.read_exactly(buf, sizeof(buf));
                    client.write(buf, sizeof(buf));
                }
                client.close();
            }, std::move(server_client));
        }
    });

    std::vector<fiberio::socket> clients(num_clients);
    for (auto& client : clients) {
        client.connect(server.get_host(), server.get_port());
    }

    this_fiber::sleep_for(std::chrono::microseconds{10});

    time_measure measure;
    std::vector<fibers::future<void>> futures(num_clients);
    for (int i = 0; i < num_clients; i++) {
        futures.at(i) = fibers::async([](fiberio::socket client) {
            char buf[] {'a'};
            char buf2[1];
            for (uint64_t i = 0; i < num_iterations; i++) {
                client.write(buf, sizeof(buf));
                client.read_exactly(buf2, sizeof(buf2));
            }
            client.close();
        }, std::move(clients.at(i)));
    }
    for (int i = 0; i < num_clients; i++) {
        futures.at(i).get();
    }
    measure.finish(num_iterations * num_clients);

    server_future.get();
    server.close();
}

void bench_fiber_switching()
{
    const uint64_t num_iterations{ 1000'000 };

    int shared_int = 0;
    dummy_lock lock;
    fibers::condition_variable_any cond;

    fibers::async([&]() {
        for (uint64_t i = 0; i < num_iterations; i++) {
            while (shared_int != 1) {
                cond.wait(lock);
            }
            shared_int = 0;
            cond.notify_one();
        }
    });

    time_measure measure;
    for (uint64_t i = 0; i < num_iterations; i++) {
        while (shared_int != 0) {
            cond.wait(lock);
        }
        shared_int = 1;
        cond.notify_one();
    }
    while (shared_int != 0) {
        cond.wait(lock);
    }
    measure.finish(2 * num_iterations);
}

int main()
{
    std::cout << "bench_echo_one_byte\n";
    std::async(bench_echo_one_byte).get();

    std::cout << "\nbench_fiber_switching\n";
    std::async(bench_fiber_switching).get();
    return 0;
}
