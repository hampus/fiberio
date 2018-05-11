#ifndef _FIBERIO_SOCKET_H_
#define _FIBERIO_SOCKET_H_

#include <memory>
#include <string>

namespace fiberio {

class socket_impl;

class socket
{
public:
    static constexpr std::size_t DEFAULT_BUF_SIZE = 256 * 1024;

    socket();

    socket(socket&&);

    socket(std::unique_ptr<socket_impl>&& impl);

    ~socket();

    void connect(const std::string& host, uint16_t port);

    std::size_t read(char* buf, std::size_t size);

    void read_exactly(char* buf, std::size_t size);

    std::string read_string(std::size_t count = DEFAULT_BUF_SIZE,
        bool shrink_to_fit = true);

    std::string read_string_exactly(std::size_t count);

    void write(const char* data, std::size_t len);

    void write(const std::string& data);

    void close();

private:
    std::unique_ptr<socket_impl> impl_;
};


}

#endif
