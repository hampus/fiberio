#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <cstdint>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

int main()
{
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 5500);
    server.listen(50);

    const uint64_t iterations = 100000;

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();

        auto start = std::chrono::high_resolution_clock::now();
        char buf[10];
        for (uint64_t i = 0; i < iterations; i++) {
            server_client.read(buf, sizeof(buf));
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = end - start;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>
            (duration).count();
        double microsecs_per_iter =
            static_cast<double>(ns) / iterations / 1000.0;
        double seconds_per_iter =
            static_cast<double>(ns) / iterations / 1000000000.0;
        double iters_per_sec = 1.0 / seconds_per_iter;
        std::cout << "nanoseconds: " << ns << "\n";
        std::cout << "per iteration: " << microsecs_per_iter <<
            " microseconds\n";
        std::cout << "iterations per second: " << iters_per_sec << "\n";
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    char buf[] {"a"};
    for (uint64_t i = 0; i < iterations; i++) {
        client.write(buf, sizeof(buf));
    }

    server_future.get();

    client.close();
    server.close();

    std::cout << "exiting main()\n";
    return 0;
}
