#include <fiberio/tcpsocket.hpp>
#include <fiberio/detail/utils.hpp>
#include <boost/fiber/all.hpp>
#include <iostream>
#include <mutex>
#include <vector>
#include <uv.h>

namespace fibers = boost::fibers;

namespace fiberio {

namespace {

const bool DEBUG_LOG = false;

const unsigned int BUF_SIZE = 256*1024;
std::vector<char> g_buf(BUF_SIZE);
bool g_buf_used = false;

class socket_impl : public tcpsocket
{
public:
    socket_impl();
    ~socket_impl();

    void bind(const addrinfo_ptr& addr);
    void listen(int backlog);
    std::unique_ptr<tcpsocket> accept();
    std::string read();
    void write(const std::string& data);

    void on_connection();
    void do_accept(socket_impl& server);
    void on_read(std::string&& data);
private:
    uv_loop_t* loop_;
    uv_tcp_t tcp_;
    fibers::mutex mutex_;
    fibers::condition_variable cond_;
    int pending_connections_;
    std::string in_buf_;
};

void connection_callback(uv_stream_t* server, int status)
{
    void* data = uv_handle_get_data((uv_handle_t*) server);
    socket_impl* socket = static_cast<socket_impl*>(data);
    socket->on_connection();
}

void alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    if (g_buf_used) {
        if (DEBUG_LOG) std::cout << "allocating buffer of " <<
            suggested_size << " bytes\n";
        buf->base = static_cast<char*>(malloc(suggested_size));
        buf->len = suggested_size;
    } else {
        if (DEBUG_LOG) std::cout << "using global buffer\n";
        buf->base = g_buf.data();
        buf->len = g_buf.size();
        g_buf_used = true;
    }
}

void read_callback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    void* data = uv_handle_get_data((uv_handle_t*) stream);
    socket_impl* socket = static_cast<socket_impl*>(data);

    std::string data_string(buf->base, nread);

    if (buf->base == g_buf.data()) {
        if (DEBUG_LOG) std::cout << "freeing global buffer\n";
        g_buf_used = false;
    } else {
        if (DEBUG_LOG) std::cout << "freeing buffer\n";
        free(buf->base);
    }

    socket->on_read(std::move(data_string));
}

void write_callback(uv_write_t* req, int status)
{
    void* data = uv_req_get_data((uv_req_t*) req);
    fibers::promise<void>* promise = static_cast<fibers::promise<void>*>(data);
    promise->set_value();
}

socket_impl::socket_impl()
{
    if (DEBUG_LOG) std::cout << "creating tcp_socket\n";
    loop_ = uv_default_loop();
    pending_connections_ = 0;
    uv_tcp_init(loop_, &tcp_);
    uv_handle_set_data((uv_handle_t*) &tcp_, this);
}

socket_impl::~socket_impl()
{
    if (DEBUG_LOG) std::cout << "destroying tcp_socket\n";
    detail::close_handle(&tcp_);
}

void socket_impl::bind(const addrinfo_ptr& addr)
{
    uv_tcp_bind(&tcp_, addr->ai_addr, 0);
}

void socket_impl::listen(int backlog)
{
    uv_listen((uv_stream_t*) &tcp_, backlog, connection_callback);
}

std::unique_ptr<tcpsocket> socket_impl::accept()
{
    std::unique_lock<fibers::mutex> lock(mutex_);
    // Wait until there is at least one connection to accept
    if (DEBUG_LOG) std::cout << "waiting for connection to accept\n";
    while (pending_connections_ == 0) {
        cond_.wait(lock);
    }
    // Accept connection
    if (DEBUG_LOG) std::cout << "going to accept pending connection\n";
    pending_connections_--;
    auto socket = std::make_unique<socket_impl>();
    socket->do_accept(*this);
    return socket;
}

void socket_impl::on_connection()
{
    std::unique_lock<fibers::mutex> lock(mutex_);
    pending_connections_++;
    if (DEBUG_LOG) std::cout << "increased pending_connections_ to " <<
        pending_connections_ << "\n";
    cond_.notify_all();
}

void socket_impl::do_accept(socket_impl& server)
{
    if (DEBUG_LOG) std::cout << "accepting connection\n";
    uv_accept((uv_stream_t*) &server.tcp_, (uv_stream_t*) &tcp_);
}

std::string socket_impl::read()
{
    std::unique_lock<fibers::mutex> lock(mutex_);

    if (in_buf_.empty()) {
        if (DEBUG_LOG) std::cout << "starting read\n";
        uv_read_start((uv_stream_t*) &tcp_, alloc_callback, read_callback);

        if (DEBUG_LOG) std::cout << "waiting for data to be read\n";
        while (in_buf_.empty()) {
            cond_.wait(lock);
        }
    }

    std::string data;
    in_buf_.swap(data);

    if (DEBUG_LOG) std::cout << "returning " << data.size() << " read bytes\n";
    return data;
}

void socket_impl::on_read(std::string&& data)
{
    std::unique_lock<fibers::mutex> lock(mutex_);
    if (DEBUG_LOG) std::cout << "read data (" << data.size() << " bytes)\n";
    if (in_buf_.empty()) {
        in_buf_.assign(data);
    } else {
        if (DEBUG_LOG) std::cout << "appending data\n";
        in_buf_.append(data);
    }
    if (!in_buf_.empty()) {
        if (DEBUG_LOG) std::cout << "stopping reading\n";
        uv_read_stop((uv_stream_t*) &tcp_);
    }
    cond_.notify_all();
}

void socket_impl::write(const std::string& data)
{
    fibers::promise<void> promise;
    uv_write_t req;
    uv_req_set_data((uv_req_t*) &req, &promise);

    // we const_cast since the API incorrectly takes a mutable char* buffer
    const uv_buf_t bufs[] {{
        .base = const_cast<char*>(data.c_str()),
        .len = data.size()
    }};

    uv_write(&req, (uv_stream_t*) &tcp_, bufs, 1, write_callback);

    promise.get_future().get();
}

}

std::unique_ptr<tcpsocket> tcpsocket::create()
{
    return std::make_unique<socket_impl>();
}

}
