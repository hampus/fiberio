#include <fiberio/socket.hpp>
#include "iobackend.hpp"

namespace fiberio {

socket::socket()
    : impl_{ std::make_unique<socket_impl>() }
{
}

socket::socket(socket&&) = default;

socket::socket(std::unique_ptr<socket_impl>&& impl)
    : impl_{ std::move(impl) }
{
}

socket::~socket()
{
}

void socket::connect(const std::string& host, uint16_t port)
{
    impl_->connect(host, port);
}

std::size_t socket::read(char* buf, std::size_t size)
{
    return impl_->read(buf, size);
}

void socket::read_exactly(char* buf, std::size_t size)
{
    std::size_t bytes_left = size;
    char* current_buf = buf;
    while (bytes_left > 0) {
        std::size_t bytes_read = read(current_buf, bytes_left);
        bytes_left -= bytes_read;
        current_buf += bytes_read;
    }
}

std::string socket::read_string(std::size_t count, bool shrink_to_fit)
{
    std::string buf(count, '\0');
    std::size_t bytes_read = read(buf.data(), buf.size());
    if (bytes_read < count) {
        buf.erase(bytes_read);
        if (shrink_to_fit) buf.shrink_to_fit();
    }
    return buf;
}

std::string socket::read_string_exactly(std::size_t count)
{
    std::string buf(count, '\0');
    read_exactly(buf.data(), buf.size());
    return buf;
}

void socket::write(const std::string& data) {
    write(data.data(), data.size());
}

void socket::write(const char* data, std::size_t len)
{
    impl_->write(data, len);
}

void socket::close()
{
    impl_->close();
}

}
