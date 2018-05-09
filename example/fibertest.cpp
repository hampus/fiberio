#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

void handle_client(std::unique_ptr<fiberio::tcpsocket> client)
{
    std::cout << "handle_client\n";
    while(true) {
        std::string data = client->read();
        if (data == "close\r\n") {
            std::cout << "closing connection\n";
            client->write("ok. closing.\n");
            return;
        } else {
            std::cout << "read " << data.size() << " bytes\n";
            if (data.empty()) break;
            std::cout << "echoing data: " << data;
            client->write(data);
        }
    }
}

int main()
{
    fiberio::use_on_this_thread();

    std::cout << "sleep_for(100)\n";
    this_fiber::sleep_for(std::chrono::milliseconds(100));
    std::cout << "returned to main\n";

    auto socket{ fiberio::tcpsocket::create() };

    std::cout << "binding socket\n";
    socket->bind("127.0.0.1", 5500);

    std::cout << "listening on socket\n";
    socket->listen(50);

    while(true) {
        std::cout << "waiting for client...\n";
        auto client = socket->accept();
        std::cout << "accepted client!\n";
        fibers::async(handle_client, std::move(client));
    }

    std::cout << "exiting main()\n";
    return 0;
}
