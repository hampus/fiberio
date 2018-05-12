FiberIO - fiber-based C++ network library
=========================================

This is an experimental C++ network library built on top of Boost.Fiber.

See [NOTICE](NOTICE) and [LICENSE](LICENSE) for license terms.

This is highly experimental at this point. Currently uses libuv internally for
asynchronous networking.

Doxygen documentation is available
[here](https://hampus.github.io/fiberio/html/).


Requirements
------------

The library is built using [Meson](http://mesonbuild.com/) and currently
depends on [Boost.Fiber](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html),
[libuv](http://libuv.org/) and (optionally)
[Google Test](https://github.com/google/googletest). It also requires C++17.

Running Ubuntu 18.04 is sufficient and then you can install dependencies with:

    $ sudo apt install build-essential meson libboost-fiber-dev libuv-dev \
            libgtest-dev


Building
--------

Use Meson:

    $ meson build
    $ cd build
    $ ninja test

This builds the library and some example applications under build/examples/. It
will also run all the unit tests.


Example
-------

A simple echo server can be created like this:

```c++
#include <fiberio/all.hpp>
#include <boost/fiber/all.hpp>

namespace fibers = boost::fibers;

int main()
{
    fiberio::use_on_this_thread();

    fiberio::server_socket server;
    server.bind("127.0.0.1", 5531);
    server.listen(50);

    while (true) {
        auto client{ server.accept() };
        fibers::async([](fiberio::socket client) {
            char buf[4096];
            while (true) {
                std::size_t bytes_read{ client.read(buf, sizeof(buf)) };
                client.write(buf, bytes_read);
            }
        }, std::move(client));
    }

    return 0;
}
```

A client application works similarly, but would create sockets like this:

```c++
fiberio::socket socket;
socket.connect("127.0.0.1", 5531);
```

Reading and writing works the same.
