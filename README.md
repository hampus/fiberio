FiberIO - fiber-based C++ network library
=========================================

[![Build Status](https://travis-ci.org/hampus/fiberio.svg?branch=master)](https://travis-ci.org/hampus/fiberio)

This is an experimental C++ network library built on top of
[Boost.Fiber](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html)
and [libuv](http://libuv.org/).

The library uses libuv internally to run multiple Boost.Fiber fibers on a single
OS thread with asynchronous I/O under the hood and cooperative scheduling
between the fibers. Fibers are lightweight threads with their own stacks. All
I/O done through the library potentially blocks the current fiber and yields to
the event loop. Fibers never yield control unless they make a blocking I/O call
or explicitly yield control through  Boost.Fiber. This can make synchronization
between fibers easier compared to multi-threaded programming, where multiple
threads run at the same time and are scheduled preemptively.

See [NOTICE](NOTICE) and [LICENSE](LICENSE) for license terms.


Details
-------

FiberIO only supports running the fibers on a single core (i.e. on one OS
thread). This means that inter-fiber synchronization is not necessary in some
cases, unless I/O is involved. A function that does no I/O and doesn't call any
other functions that might yield will always run to completion without any fiber
switches and can make use of that fact. In many ways this is similar to
programming with async / await in Javascript or Python, promises and futures,
NodeJS, or even explicit event-based programming using libuv, libevent, epoll,
kqueue, or IOCP on a single thread. It's just easier and more natural.

From another point-of-view, it also looks like writing traditional threaded
source code with blocking I/O calls. It's also possible to call libraries that
are unaware of fibers. The main potential problem is that if something blocks
the thread that the fiber runs on, no other fiber will be able to run. Blocking
calls should only be made through this library (or Boost.Fiber) so that they
block only the fiber and yield control to the event loop behind the scenes. This
is a familiar limitation to anyone who has done event-based single-threaded
programming using other systems.

The programming model offered by FiberIO can be convenient where running on a
single CPU core is sufficient and when it's either feasible to avoid doing
anything that would block the whole thread or it's fine to accept the
performance penalty of doing so. When it's necessary to make use of multiple CPU
cores or when it's useful to do anything that might block the whole thread
(memory mapped I/O or e.g. using libraries that are unaware of fibers and might
do something thread-blocking), it's generally recommended to make use of 1:1 OS
threads (possibly with some degree of user space scheduling). That is
out-of-scope for this library.

It's not yet possible to communicate between fibers and other threads, but that
will be possible in the future. The fibers running on a thread will always be
bound to a single thread, however, as part of the design of FiberIO. Manually
offloading some specific tasks to external thread pools and having fibers wait
for the results can be a good way to make use of multiple CPU cores for tasks
that need it, while still being able to use FiberIO for the rest.

In theory, it's possible to run FiberIO on multiple threads and having fibers
scheduled completely independently on each thread. That works perfectly fine,
but it's not the intended use case. Communication between fibers on different
threads work the same as communicating with other threads generally (see above).
Fibers will never be migrated between threads.


Documentation
-------------

Doxygen documentation is available
[here](https://hampus.github.io/fiberio/html/).


Requirements
------------

The library is built using [Meson](http://mesonbuild.com/) and currently
depends on [Boost.Fiber](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html),
[libuv](http://libuv.org/) and (optionally)
[Google Test](https://github.com/google/googletest). It also requires C++17.

On Ubuntu 18.04, you can install dependencies with:

    $ sudo apt install wget build-essential automake libtool pkg-config meson \
        libboost-fiber-dev libgtest-dev

You also need libuv, but the version in Ubuntu 18.04 is too old so you'll need
to build and install it manually from source.

On Arch Linux, you can install everything using pacman:

    $ sudo pacman -S  pkg-config meson boost libuv gtest


Building
--------

Use Meson:

    $ meson build
    $ cd build
    $ ninja test

This builds the library and some example applications under build/examples/. It
will also run all the unit tests.

Valgrind
--------

If you built your Boost library with
[valgrind=on](https://www.boost.org/doc/libs/release/libs/context/doc/html/context/stack/valgrind.html)
set, you can enable the valgrind build option for fiberio to avoid valgrind
warnings.

Either add it when creating the build directory:

    $ meson -Dvalgrind=true build

or change the Meson configuration later:

    $ cd build
    $ meson configure -Dvalgrind=true

Older versions of Meson works differently (read the manual).


Building with Docker
--------------------

For testing purposes, there are some Docker files and scripts for building
inside Docker containers.

You can build with all of those by running (from this directory):

    $ ./ci/buildall.sh

This builds both in Ubuntu 18.04 and Arch Linux and using GCC and Clang.


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
