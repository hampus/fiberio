#include "socket_impl.hpp"
#include <fiberio/exceptions.hpp>
#include "addrinfo.hpp"
#include "loop.hpp"
#include "utils.hpp"
#include <exception>

namespace fibers = boost::fibers;

namespace fiberio {

namespace {

const bool DEBUG_LOG = false;

const int64_t ERROR_EOF = -1;
const int64_t ERROR_READ_FAILED = -2;

void connection_callback(uv_connect_t* req, int status)
{
    void* data = uv_req_get_data((uv_req_t*) req);
    fibers::promise<void>* promise = static_cast<fibers::promise<void>*>(data);
    try {
        check_uv_status(status);
        promise->set_value();
    } catch (std::exception& e) {
        promise->set_exception(std::current_exception());
    }
}

void write_callback(uv_write_t* req, int status)
{
    void* data = uv_req_get_data((uv_req_t*) req);
    fibers::promise<void>* promise = static_cast<fibers::promise<void>*>(data);
    try {
        check_uv_status(status);
        promise->set_value();
    } catch (std::exception& e) {
        promise->set_exception(std::current_exception());
    }
}

void alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    void* data = uv_handle_get_data((uv_handle_t*) handle);
    socket_impl* socket = static_cast<socket_impl*>(data);

    if (DEBUG_LOG) std::cout << "using buffer from socket of " <<
        socket->get_len() << " bytes\n";
    buf->base = socket->get_buf();
    buf->len = socket->get_len();
}

void read_callback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    void* data = uv_handle_get_data((uv_handle_t*) stream);
    socket_impl* socket = static_cast<socket_impl*>(data);

    if (DEBUG_LOG) std::cout << "stop reading\n";
    uv_read_stop(stream);

    if (nread >= 0) {
        if (DEBUG_LOG) std::cout << "read " << nread << " bytes\n";
        socket->on_read_finished(nread);
    } else if (nread == UV_EOF) {
        socket->on_read_finished(ERROR_EOF);
    } else {
        if (DEBUG_LOG) std::cout << "read error\n";
        socket->on_read_finished(ERROR_READ_FAILED);
    }
}

void shutdown_callback(uv_shutdown_t* req, int status)
{
    void* data = uv_req_get_data((uv_req_t*) req);
    fibers::promise<void>* promise = static_cast<fibers::promise<void>*>(data);
    try {
        check_uv_status(status);
        promise->set_value();
    } catch (std::exception& e) {
        promise->set_exception(std::current_exception());
    }
}

}

socket_impl::socket_impl()
    : loop_{get_uv_loop()}, closed_{false}, reading_{false}, buf_{0}, len_{0}
{
    if (DEBUG_LOG) std::cout << "creating socket_impl\n";
    uv_tcp_init(loop_, &tcp_);
    uv_handle_set_data((uv_handle_t*) &tcp_, this);
}

socket_impl::~socket_impl() {
    if (!closed_) {
        if (DEBUG_LOG) std::cout <<
            "closing socket_impl from destructor\n";
        close();
    } else {
        if (DEBUG_LOG) std::cout <<
            "destroying closed socket_impl\n";
    }
}

void socket_impl::do_accept(uv_stream_t* server)
{
    if (DEBUG_LOG) std::cout << "accepting connection\n";
    int status = uv_accept(server, (uv_stream_t*) &tcp_);
    check_uv_status(status);
}

void socket_impl::connect(const std::string& host, uint16_t port)
{
    if (closed_) throw socket_closed_error{};
    if (DEBUG_LOG) std::cout << "calling getaddrinfo for " << host << ":" <<
        port << "\n";
    try {
        auto addr = getaddrinfo(host, port);
        if (DEBUG_LOG) std::cout << "connecting to " << host << ":" <<
            port << "\n";
        fibers::promise<void> promise;
        uv_connect_t req;
        uv_req_set_data((uv_req_t*) &req, &promise);
        int status = uv_tcp_connect(&req, &tcp_, addr->ai_addr,
            connection_callback);
        check_uv_status(status);
        wait_for_future(promise.get_future());
    } catch (uv_error& e) {
        throw io_error{ e.what() };
    }
}

std::size_t socket_impl::read(char* buf, std::size_t size)
{
    if (closed_) throw socket_closed_error{};
    if (reading_) {
        if (DEBUG_LOG) std::cout << "socket_impl: concurrent read\n";
        throw std::runtime_error("concurrent read");
    }
    reading_ = true;
    buf_ = buf;
    len_ = size;

    try {
        if (DEBUG_LOG) std::cout << "starting read\n";
        int status =
            uv_read_start((uv_stream_t*) &tcp_, alloc_callback, read_callback);
        check_uv_status(status);
        wait_for_read_to_finish();
        reading_ = false;

        if (len_ == ERROR_EOF) {
            close();
            throw socket_closed_error();
        } else if (len_ == ERROR_READ_FAILED) {
            throw std::runtime_error("read failed");
        }

        return len_;
    } catch (std::exception& e) {
        buf_ = 0;
        reading_ = false;
        throw;
    }
}

void socket_impl::wait_for_read_to_finish()
{
    if (DEBUG_LOG) std::cout << "waiting for read to finish\n";
    dummy_lock lock;
    while (buf_ != 0 && !closed_) {
        cond_.wait(lock);
    }
}

void socket_impl::on_read_finished(ssize_t nread)
{
    buf_ = 0;
    len_ = nread;
    cond_.notify_all();
}

void socket_impl::write(const char* data, std::size_t len)
{
    if (closed_) throw socket_closed_error{};
    fibers::promise<void> promise;
    uv_write_t req;
    uv_req_set_data((uv_req_t*) &req, &promise);

    // we const_cast since the API incorrectly takes a mutable char* buffer
    const uv_buf_t bufs[] {{
        .base = const_cast<char*>(data),
        .len = len
    }};

    if (DEBUG_LOG) std::cout << "starting write of " << len << " bytes\n";
    int status = uv_write(&req, (uv_stream_t*) &tcp_, bufs, 1, write_callback);
    check_uv_status(status);

    wait_for_future(promise.get_future());
    if (DEBUG_LOG) std::cout << "write finished\n";
}

void socket_impl::close()
{
    if (!closed_) {
        closed_ = true;
        if (DEBUG_LOG) std::cout << "closing socket_impl\n";
        try {
            shutdown();
        } catch (socket_closed_error& e) {
            if (DEBUG_LOG) std::cout << "Closing disconnected socket\n";
        }
        close_handle(&tcp_);
        cond_.notify_all();
    }
}

void socket_impl::shutdown()
{
    fibers::promise<void> promise;
    uv_shutdown_t req;
    uv_req_set_data((uv_req_t*) &req, &promise);
    int status = uv_shutdown(&req, (uv_stream_t*) &tcp_, shutdown_callback);
    check_uv_status(status);
    wait_for_future(promise.get_future());
}

}
