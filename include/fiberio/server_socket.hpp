#ifndef _FIBERIO_SERVER_SOCKET_H_
#define _FIBERIO_SERVER_SOCKET_H_

#include <fiberio/socket.hpp>
#include <memory>
#include <string>
#include <cstdint>

namespace fiberio {

class server_socket_impl;

class server_socket
{
public:
    server_socket();

    server_socket(server_socket&& other);

    ~server_socket();

    void bind(const std::string& host, uint16_t port);

    std::string get_host();

    uint16_t get_port();

    void listen(int backlog);

    socket accept();

    void close();

private:
    std::unique_ptr<server_socket_impl> impl_;
};


}

#endif
