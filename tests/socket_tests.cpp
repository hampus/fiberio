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

TEST(server_socket, connect_sockets_ipv6) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("::1", 0);
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

TEST(server_socket, closed_socket) {
    fiberio::use_on_this_thread();
    fiberio::socket client;
    ASSERT_THROW(client.read_string(), fiberio::socket_closed_error);
    ASSERT_THROW(client.read_string(), fiberio::socket_closed_error);
    ASSERT_THROW(client.write("test"), fiberio::socket_closed_error);
    client.close();
    ASSERT_THROW(client.read_string(), fiberio::socket_closed_error);
    ASSERT_THROW(client.read_string(), fiberio::socket_closed_error);
    ASSERT_THROW(client.write("test"), fiberio::socket_closed_error);
    ASSERT_THROW(client.connect("127.0.0.1", 1000),
        fiberio::socket_closed_error);
}

TEST(server_socket, destroy_new_socket) {
    fiberio::use_on_this_thread();
    fiberio::socket client;
}

TEST(server_socket, destroy_new_server_socket) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
}

TEST(server_socket, address_family_not_supported_error) {
    fiberio::use_on_this_thread();
    fiberio::address_family_not_supported_error e;
    ASSERT_EQ("address family not supported", std::string(e.what()));
}

TEST(server_socket, fail_to_connect) {
    fiberio::use_on_this_thread();

    fiberio::socket client;
    ASSERT_THROW(client.connect("!", 123), fiberio::io_error);
    ASSERT_THROW(client.connect("0.0.0.0", 123), fiberio::io_error);
    ASSERT_THROW(client.connect("::", 123), fiberio::io_error);
    ASSERT_THROW(client.connect("127.0.0.1", 0), fiberio::io_error);
    ASSERT_THROW(client.connect("127.0.0.1", -1), fiberio::io_error);
    ASSERT_THROW(client.connect("127.0.0.1", 1), fiberio::io_error);
}

TEST(server_socket, read_past_eof) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        server_client.write("abc");
        server_client.close();
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    server_future.get();
    server.close();

    std::string data = client.read_string_exactly(3);
    ASSERT_TRUE(client.is_open());
    client.read_string(1);
    ASSERT_FALSE(client.is_open());
    // We have been warned (i.e. is_open() is false already) so it throws
    ASSERT_THROW(client.read_string(1), fiberio::socket_closed_error);

    client.close();
}

TEST(server_socket, read_exactly_past_eof) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        server_client.write("abc");
        server_client.close();
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    server_future.get();
    server.close();

    std::string data = client.read_string_exactly(3);
    ASSERT_THROW(client.read_string_exactly(1), fiberio::socket_closed_error);

    client.close();
}


TEST(server_socket, write_after_other_end_closed) {
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        server_client.write("abc");
        server_client.close();
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    server_future.get();
    server.close();

    client.write("a");

    ASSERT_THROW(client.read_string_exactly(4), fiberio::socket_closed_error);

    client.close();
}

TEST(server_socket, large_write) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        std::string data;
        while (server_client.is_open()) {
            data += server_client.read_string();
        }
        return data;
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    std::string data_to_write(1024*1024, 't');
    client.write(data_to_write);
    client.close();

    ASSERT_EQ(data_to_write, server_future.get());

    server.close();
}

TEST(server_socket, concurrent_reads_and_close) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        server_client.read_string();
        ASSERT_FALSE(server_client.is_open());
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());

    auto client_future1 = fibers::async([&client]() {
        client.read_string();
        ASSERT_FALSE(client.is_open());
    });

    auto client_future2 = fibers::async([&client]() {
        // This is not recommended in application code!
        ASSERT_THROW(client.read_string(), fiberio::io_error);
    });

    client_future2.get();
    client.close();
    client_future1.get();
    server_future.get();
    server.close();
}

TEST(server_socket, partial_string_read) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    const std::size_t buf_size{ 256*1024 };
    auto server_future = fibers::async([&server]() {
        auto server_client = server.accept();
        auto s1 = server_client.read_string(buf_size);
        return s1;
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    client.write("0123456789abcdefghijklmnopqrstuvx");

    auto result = server_future.get();
    ASSERT_EQ("0123456789abcdefghijklmnopqrstuvx", result);

    // The C++ string implementation is allowed to leave the capacity larger
    // than the contents, but we want to check that it has been shrunk at least
    ASSERT_LT(result.capacity(), buf_size);

    client.close();
    server.close();
}

TEST(server_socket, iostream_read_write) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        fiberio::socket_stream stream{ server.accept() };
        int number;
        stream >> number;
        return number;
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    fiberio::socket_stream stream{client};

    stream << 56567;
    stream.close();

    auto result = server_future.get();
    ASSERT_EQ(56567, result);

    server.close();
}

TEST(server_socket, connect_stream) {
    fiberio::use_on_this_thread();
    fiberio::server_socket server;
    server.bind("127.0.0.1", 0);
    server.listen(50);

    auto server_future = fibers::async([&server]() {
        fiberio::socket_stream stream{ server.accept() };
        int number;
        stream >> number;
        return number;
    });

    fiberio::socket client;
    client.connect(server.get_host(), server.get_port());
    fiberio::socket_stream stream{ client };

    stream << 56567;
    stream.close();

    auto result = server_future.get();
    ASSERT_EQ(56567, result);

    server.close();
}
