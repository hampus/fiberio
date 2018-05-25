#ifndef _FIBERIO_IOSTREAM_HPP_
#define _FIBERIO_IOSTREAM_HPP_

#include <iostream>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>

namespace fiberio {

namespace detail {

class socket_device :
    public boost::iostreams::device<boost::iostreams::bidirectional>
{
public:
    socket_device(socket socket)
        : socket_{ socket }, closed_in_{false}, closed_out_{false}
    {}

    std::streamsize read(char* s, std::streamsize n) {
        std::streamsize bytes_read = 0;
        while(socket_.is_open() && bytes_read < n) {
            bytes_read += socket_.read(s + bytes_read, n - bytes_read);
        }
        return bytes_read;
    }

    std::streamsize write(const char* s, std::streamsize n) {
        socket_.write(s, n);
        return n;
    }

    void close(std::ios_base::openmode mode) {
        if (mode == std::ios_base::out) {
            closed_out_ = true;
        } else if (mode == std::ios_base::in) {
            closed_in_ = true;
        }
        if (closed_in_ && closed_out_) {
            socket_.close();
        }
    }

private:
    socket socket_;
    bool closed_in_ : 1;
    bool closed_out_ : 1;
};

}

/*! \brief std::basic_iostream type for reading/writing to a socket
 *
 * Takes a fiberio::socket as constructor argument. See the C++ standard for how
 * to use a basic_iostream and Boost.IOStreams for implementation details.
 */
using socket_stream = boost::iostreams::stream<fiberio::detail::socket_device>;

/*! \brief std::basic_streambuf type for reading/writing to a socket
 *
 * Takes a fiberio::socket as constructor argument. See the C++ standard for how
 * to use a basic_streambuf and Boost.IOStreams for implementation details.
 */
using socket_streambuf =
    boost::iostreams::stream_buffer<fiberio::detail::socket_device>;

//! Create a connected socket stream directly
inline socket_stream connect_stream(const std::string& host, uint16_t port) {
    socket socket;
    socket.connect(host, port);
    return socket_stream{ std::move(socket) };
}

}

#endif
