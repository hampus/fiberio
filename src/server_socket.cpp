#include <fiberio/server_socket.hpp>
#include "iobackend.hpp"

namespace fiberio {

server_socket::server_socket()
    : impl_{ std::make_unique<server_socket_impl>() }
{
}

server_socket::server_socket(server_socket&& other) = default;

server_socket::~server_socket()
{
}

void server_socket::bind(const std::string& host, uint16_t port)
{
    impl_->bind(host, port);
}

std::string server_socket::get_host()
{
    return impl_->get_host();
}

uint16_t server_socket::get_port()
{
    return impl_->get_port();
}

void server_socket::listen(int backlog)
{
    impl_->listen(backlog);
}

socket server_socket::accept()
{
    return impl_->accept();
}

void server_socket::close()
{
    impl_->close();
}

}
