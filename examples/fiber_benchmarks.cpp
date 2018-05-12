#include "time_measure.hpp"
#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <cstdint>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <vector>
#include <cstring>
#include <cerrno>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>

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

    const uint64_t num_iterations{ 50 };
    const int num_clients{ 1000 };

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

void check_result(int result)
{
    if (result < 0) {
        std::cout << "Error: " << std::strerror(errno) << "\n";
        std::terminate();
    }
}

void bench_echo_one_byte_ideal_unix_socket_pair()
{
    const uint64_t num_iterations{ 50 };
    const int num_clients{ 1000 };

    std::vector<int> server_fds;
    std::vector<int> client_fds;
    for (int i = 0; i < num_clients; i++) {
        int fd[2];
        check_result(socketpair(AF_UNIX, SOCK_STREAM, 0, fd));
        server_fds.push_back(fd[0]);
        client_fds.push_back(fd[1]);
    }

    char buf1[] {'a'};
    char buf2[1];
    time_measure measure;
    for (uint64_t i = 0; i < num_iterations; i++) {
        for (auto client_fd : client_fds) {
            check_result(send(client_fd, buf1, 1, 0));
        }
        for (auto server_fd : server_fds) {
            check_result(recv(server_fd, buf2, 1, 0));
            check_result(send(server_fd, buf2, 1, 0));
        }
        for (auto client_fd : client_fds) {
            check_result(recv(client_fd, buf2, 1, 0));
        }
    }
    measure.finish(num_iterations * num_clients);
}


void bench_echo_one_byte_raw_threaded_unix_socket_pair()
{
    const uint64_t num_iterations{ 50 };
    const int num_clients{ 1000 };

    std::vector<int> server_fds;
    std::vector<int> client_fds;
    for (int i = 0; i < num_clients; i++) {
        int fd[2];
        check_result(socketpair(AF_UNIX, SOCK_STREAM, 0, fd));
        server_fds.push_back(fd[0]);
        client_fds.push_back(fd[1]);
    }

    std::vector<std::future<void>> server_futures(num_clients);
    for (int i = 0; i < num_clients; i++) {
        server_futures.at(i) = std::async(std::launch::async,
            [](int fd) {
                char buf[1];
                for (uint64_t i = 0; i < num_iterations; i++) {
                    check_result(recv(fd, buf, 1, 0));
                    check_result(send(fd, buf, 1, 0));
                }
            }, server_fds.at(i));
    }

    time_measure measure;
    std::vector<std::future<void>> client_futures(num_clients);
    for (int i = 0; i < num_clients; i++) {
        client_futures.at(i) = std::async(std::launch::async,
            [](int fd) {
                char buf1[] {'a'};
                char buf2[1];
                for (uint64_t i = 0; i < num_iterations; i++) {
                    check_result(send(fd, buf1, 1, 0));
                    check_result(recv(fd, buf2, 1, 0));
                }
            }, client_fds.at(i));
    }
    for (int i = 0; i < num_clients; i++) {
        client_futures.at(i).get();
    }
    measure.finish(num_iterations * num_clients);

    for (int i = 0; i < num_clients; i++) {
        server_futures.at(i).get();
    }
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

void bench_thread_switching()
{
    const uint64_t num_iterations{ 100'000 };

    int shared_int = 0;
    std::mutex mutex;
    std::condition_variable cond;

    auto future = std::async(std::launch::async, [&]() {
        for (uint64_t i = 0; i < num_iterations; i++) {
            std::unique_lock<std::mutex> lock(mutex);
            while (shared_int != 1) {
                cond.wait(lock);
            }
            shared_int = 0;
            cond.notify_one();
        }
    });

    time_measure measure;
    std::unique_lock<std::mutex> lock(mutex);
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
    future.get();
}

int main()
{
    std::cout << "bench_echo_one_byte\n";
    std::async(bench_echo_one_byte).get();

    std::cout << "\nbench_echo_one_byte_ideal_unix_socket_pair\n";
    std::async(bench_echo_one_byte_ideal_unix_socket_pair).get();

    std::cout << "\nbench_echo_one_byte_raw_threaded_unix_socket_pair\n";
    std::async(bench_echo_one_byte_raw_threaded_unix_socket_pair).get();

    std::cout << "\nbench_fiber_switching\n";
    std::async(bench_fiber_switching).get();

    std::cout << "\nbench_thread_switching\n";
    std::async(bench_thread_switching).get();
    return 0;
}
