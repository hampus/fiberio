#include "server_socket_impl.hpp"
#include "socket_impl.hpp"
#include "addrinfo.hpp"
#include "loop.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace fibers = boost::fibers;

namespace fiberio {

namespace {

const bool DEBUG_LOG = false;

void connection_callback(uv_stream_t* server, int status)
{
    void* data = uv_handle_get_data((uv_handle_t*) server);
    server_socket_impl* socket = static_cast<server_socket_impl*>(data);
    socket->on_connection(status);
}

std::string addr_to_string(int af, const void* src)
{
    if (af != AF_INET && af != AF_INET6) {
        throw std::runtime_error("Unknown address family");
    }
    char buf[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
    auto result = inet_ntop(af, src, buf, sizeof(buf));
    if (!result) throw std::runtime_error("inet_ntop failed");
    return std::string(buf);
}

}

server_socket_impl::server_socket_impl()
    : loop_{get_uv_loop()}, pending_connections_{0}, closed_{false}
{
    if (DEBUG_LOG) std::cout << "creating server_socket_impl\n";
    uv_tcp_init(loop_, &tcp_);
    uv_handle_set_data((uv_handle_t*) &tcp_, this);
}

server_socket_impl::~server_socket_impl() {
    if (!closed_) {
        if (DEBUG_LOG) std::cout <<
            "closing server_socket_impl from destructor\n";
        close();
    } else {
        if (DEBUG_LOG) std::cout <<
            "destroying closed server_socket_impl\n";
    }
}

void server_socket_impl::bind(const std::string& host, uint16_t port) {
    if (DEBUG_LOG) std::cout << "calling getaddrinfo for " << host <<
        ":" << port << "\n";
    host_ = host;
    port_ = port;
    auto addr = getaddrinfo(host, port);
    if (DEBUG_LOG) std::cout << "binding to address\n";
    int status = uv_tcp_bind(&tcp_, addr->ai_addr, 0);
    check_uv_status(status);
    update_address();
}

void server_socket_impl::update_address()
{
    struct sockaddr_storage addr;
    int addrlen = sizeof(addr);
    int status = uv_tcp_getsockname(&tcp_, (struct sockaddr*) &addr, &addrlen);
    check_uv_status(status);
    if (addr.ss_family == AF_INET) {
        sockaddr_in* addr4 = (sockaddr_in*) &addr;
        host_ = addr_to_string(AF_INET, &addr4->sin_addr);
        port_ = ntohs(addr4->sin_port);
    } else if (addr.ss_family == AF_INET6) {
        sockaddr_in6* addr6 = (sockaddr_in6*) &addr;
        host_ = addr_to_string(AF_INET6, &addr6->sin6_addr);
        port_ = ntohs(addr6->sin6_port);
    } else {
        if (DEBUG_LOG) std::cout << "Unknown address family\n";
    }
    if (DEBUG_LOG) {
        std::cout << "bound to " << host_ << ":" << port_ << "\n";
    }
}

std::string server_socket_impl::get_host()
{
    return host_;
}

uint16_t server_socket_impl::get_port()
{
    return port_;
}

void server_socket_impl::listen(int backlog) {
    if (DEBUG_LOG) std::cout << "listening\n";
    int status = uv_listen((uv_stream_t*) &tcp_, backlog, connection_callback);
    check_uv_status(status);
}

void server_socket_impl::on_connection(int status) {
    try {
        pending_connections_++;
        if (DEBUG_LOG) std::cout << "increased pending_connections_ to " <<
            pending_connections_ << "\n";
        cond_.notify_all();
    } catch (std::exception& e) {
        if (DEBUG_LOG) std::cout << "on_connection error: " <<
            e.what() << "\n";
        close();
    }
}

socket server_socket_impl::accept() {
    std::unique_lock<fibers::mutex> lock(mutex_);
    // Wait until there is at least one connection to accept
    if (DEBUG_LOG) std::cout << "waiting for connection to accept\n";
    while (pending_connections_ == 0 && !closed_) {
        cond_.wait(lock);
    }
    if (closed_) throw std::runtime_error("connection closed");
    // Accept connection
    if (DEBUG_LOG) std::cout << "going to accept pending connection\n";
    pending_connections_--;
    auto new_socket_impl = std::make_unique<socket_impl>();
    new_socket_impl->do_accept((uv_stream_t*) &tcp_);
    return socket{ std::move(new_socket_impl) };
}

void server_socket_impl::close() {
    if (!closed_) {
        if (DEBUG_LOG) std::cout << "closing server_socket_impl\n";
        closed_ = true;
        close_handle(&tcp_);
        cond_.notify_all();
    }
}

}
