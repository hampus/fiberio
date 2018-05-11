#ifndef _FIBERIO_SRC_SERVER_SOCKET_IMPL_H_
#define _FIBERIO_SERVER_SOCKET_H_

#include <fiberio/socket.hpp>
#include <boost/fiber/all.hpp>
#include <memory>
#include <vector>
#include <thread>
#include <cstdint>
#include <uv.h>

namespace fiberio {

class server_socket_impl
{
public:
    server_socket_impl();

    ~server_socket_impl();

    server_socket_impl(const server_socket_impl&) = delete;
    server_socket_impl(server_socket_impl&&) = delete;
    server_socket_impl& operator=(const server_socket_impl&) = delete;
    server_socket_impl& operator=(server_socket_impl&&) = delete;

    void bind(const std::string& host, uint16_t port);

    void update_address();

    std::string get_host();

    uint16_t get_port();

    void listen(int backlog);

    void on_connection(int status);

    socket accept();

    void close();

private:
    uv_loop_t* loop_;
    uv_tcp_t tcp_;
    boost::fibers::condition_variable_any cond_;
    int pending_connections_;
    bool closed_;
    std::string host_;
    uint16_t port_;
};


}

#endif
