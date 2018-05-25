#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>

namespace fibers = boost::fibers;

int main()
{
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 5531);
    server.listen(50);

    while (true) {
        fibers::async([](fiberio::socket client) {
            char buf[4096];
            while (client.is_open()) {
                std::size_t bytes_read{ client.read(buf, sizeof(buf)) };
                client.write(buf, bytes_read);
            }
        }, server.accept());
    }

    return 0;
}
