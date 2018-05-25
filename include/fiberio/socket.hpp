#ifndef _FIBERIO_SOCKET_H_
#define _FIBERIO_SOCKET_H_

#include <memory>
#include <string>

namespace fiberio {

class socket_impl;

//! Client socket for communicating over a network and opening connections
class socket
{
public:
    static constexpr std::size_t DEFAULT_BUF_SIZE = 256 * 1024;

    //! Creates a non-connected socket
    socket();

    //! Creates a socket based on another, which will be invalid after
    socket(socket&&);

    //! For internal use only
    socket(std::unique_ptr<socket_impl>&& impl);

    //! Destructor. Closes the socket if still open.
    ~socket();

    //! Connects to host:port and throws an exception on failure.
    void connect(const std::string& host, uint16_t port);

    /*! \brief Reads up to size bytes into buf.
     *
     * Throws an exception on failure. fiberio::socket_closed_error is thrown
     * if the socket was already closed. If the end-of-stream is encountered
     * during the read, no exception is thrown unless another read call is made.
     */
    std::size_t read(char* buf, std::size_t size);

    //! The same as read() but always fills the buffer completely (or fails)
    void read_exactly(char* buf, std::size_t size);

    /*! \brief Reads up to count bytes and returns it as an std::string
     *
     * Allocates a buffer of size count. If shrink_to_fit is true, it will
     * free unused buffer space before returning the string. Otherwise, it will
     * return a string with a capacity of count bytes.
     *
     * This works the same as read() except that it returns a string.
     */
    std::string read_string(std::size_t count = DEFAULT_BUF_SIZE,
        bool shrink_to_fit = true);

    //! The same as read_string() but reads exactly count bytes (or fails)
    std::string read_string_exactly(std::size_t count);

    //! Writes data from the buffer and returns once the buffer can be freed
    void write(const char* data, std::size_t len);

    //! Writes data from the buffer and returns once the buffer can be freed
    void write(const std::string& data);

    /*! \brief Closes the socket if it's not already closed.
     *
     * It's safe to call this repeatedly as it's idempotent.
     */
    void close();

    /*! \brief Check if the connection is open
     *
     * This starts out true and changes when the socket is closed or discovers
     * that the underlying connection was closed.
     */
    bool is_open();

private:
    std::unique_ptr<socket_impl> impl_;
};


}

#endif
