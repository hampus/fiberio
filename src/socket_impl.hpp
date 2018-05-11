#ifndef _FIBERIO_SRC_SOCKET_IMPL_H_
#define _FIBERIO_SRC_SOCKET_IMPL_H_

#include <boost/fiber/all.hpp>
#include<string>
#include <uv.h>

namespace fiberio {

class socket_impl
{
public:
    socket_impl();

    ~socket_impl();

    void do_accept(uv_stream_t* server);

    void connect(const std::string& host, uint16_t port);

    std::size_t read(char* buf, std::size_t size);

    void write(const char* data, std::size_t len);

    void close();

    void on_read_finished(int64_t nread);

    char* get_buf() { return buf_; }

    int64_t get_len() { return len_; }

private:
    void wait_for_read_to_finish();

    uv_loop_t* loop_;
    uv_tcp_t tcp_;
    boost::fibers::condition_variable_any cond_;
    bool closed_ : 1;
    bool reading_ : 1;
    char* buf_;
    int64_t len_;
};

}

#endif
