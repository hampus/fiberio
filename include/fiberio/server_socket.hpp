#ifndef _FIBERIO_SERVER_SOCKET_H_
#define _FIBERIO_SERVER_SOCKET_H_

#include <fiberio/socket.hpp>
#include <memory>
#include <string>
#include <cstdint>

namespace fiberio {

class server_socket_impl;

//! Server socket for listening for incoming connections
class server_socket
{
public:
    //! Creates an unbound listening socket
    server_socket();

    //! Creates a listening socket based on another, which is invalid after.
    server_socket(server_socket&& other);

    //! Destructor. Closes the server_socket if it's open.
    ~server_socket();

    /*! \brief Binds the server_socket to an address and port
     *
     * This should be called befor listen(). Use get_host() and get_port() to
     * check what it's actually bound to after.
     *
     * A port of 0 will bind to any available port.
     */
    void bind(const std::string& host, uint16_t port);

    //! Return the host that the server_socket is bound to
    std::string get_host();

    //! Return the port that the server_socket is bound to
    uint16_t get_port();

    /*! \brief Start listening for connections with a certain backlog size
     *
     * This returns quickly and only tells the OS to start listening.
     *
     * Use accept() to accept connections after.
     */
    void listen(int backlog);

    /*! \brief Accept an incoming connection and get a socket representing it
     *
     * This is typically done repeatedly.
     *
     * It's often useful to launch a new fiber for each connection with
     * boost::fibers::async() (don't forget to std::move() the socket).
     */
    socket accept();

    //! Close the listening socket and stop listening for connections
    void close();

private:
    std::unique_ptr<server_socket_impl> impl_;
};


}

#endif
