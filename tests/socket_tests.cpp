#include <fiberio/all.hpp>
#include <gtest/gtest.h>
#include <boost/fiber/all.hpp>
#include <utility>

namespace fibers = boost::fibers;
namespace this_fiber = boost::this_fiber;

TEST(server_socket, create_server_socket) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    ASSERT_EQ("127.0.0.1", server.get_host());
    ASSERT_NE(0, server.get_port());

    server.listen(50);
    server.close();
}

TEST(server_socket, move_server_socket) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    ASSERT_EQ("127.0.0.1", server.get_host());
    ASSERT_NE(0, server.get_port());

    // Move-construction
    fiberio::server_socket server2{ std::move(server) };

    server2.listen(50);
    server2.close();
}

TEST(server_socket, connect_sockets) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        server.accept();
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    server_future.get();

    client.close();
    server.close();
}

TEST(server_socket, move_socket) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        server.accept();
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    fiberio::socket client2{ std::move(client) };

    server_future.get();

    client2.close();
    server.close();
}

TEST(server_socket, write_and_read) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        char buf[4096];
        auto bytes_read = server_client.read(buf, sizeof(buf));
        return std::string(buf, bytes_read);
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    client.write("abc");

    auto result = server_future.get();
    ASSERT_EQ("abc", result);

    client.close();
    server.close();
}

TEST(server_socket, read_exactly) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        char buf[10];
        server_client.read_exactly(buf, sizeof(buf));
        return std::string(buf, sizeof(buf));
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    client.write("test");
    client.write("123456");

    auto result = server_future.get();
    ASSERT_EQ("test123456", result);

    client.close();
    server.close();
}


TEST(server_socket, read_strings) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        auto s1 = server_client.read_string(5);
        auto s2 = server_client.read_string_exactly(15);
        return std::make_pair(s1, s2);
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    client.write("0123456789");
    client.write("abcdefghij");

    auto result = server_future.get();
    ASSERT_EQ("01234", result.first);
    ASSERT_EQ("56789abcdefghij", result.second);

    client.close();
    server.close();
}
