#include "time_measure.hpp"
#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <cstdint>
#include <future>
#include <cstring>
#include <vector>

namespace fibers = boost::fibers;

int main()
{
    fiberio::use_on_this_thread();

    const auto host{ "127.0.0.1" };
    const auto port{ 5531 };
    const auto num_iterations{ 50 };
    const auto num_clients{ 1000 };

    std::vector<fiberio::socket> clients(num_clients);
    for (auto& client : clients) {
        client.connect(host, port);
    }

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

    return 0;
}
