FiberIO - fiber-based C++ network library
=========================================

[![Travis](https://img.shields.io/travis/hampus/fiberio.svg)](https://travis-ci.org/hampus/fiberio)
[![Coveralls github](https://img.shields.io/coveralls/github/hampus/fiberio.svg)](https://coveralls.io/github/hampus/fiberio)
[![license](https://img.shields.io/github/license/hampus/fiberio.svg)](http://www.apache.org/licenses/LICENSE-2.0)

This is an experimental C++ network library built on top of
[Boost.Fiber](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html)
and [libuv](http://libuv.org/).

The library uses libuv internally to run multiple Boost.Fiber fibers on
(currently) a single OS thread with event-based I/O under the hood and
cooperative scheduling between the fibers. Fibers are lightweight threads with
their own stacks. All I/O that goes through the library might suspend the
current fiber, but won't block the underlying OS thread.

Fibers are much more lightweight than OS threads in terms of memory usage and
context switch costs, while offering a more natural programming model than e.g.
asynchronous programming with callbacks. While being cheaper on resources,
fibers also have a few limitations compared to OS threads.

See [NOTICE](NOTICE) and [LICENSE](LICENSE) for license terms.


Description
-----------

FiberIO is a Boost.Fiber scheduler that supports synchronous I/O through its own
API that suspends the calling fiber while waiting for I/O. Event-based I/O is
used behind the scenes to avoid blocking the thread, but that's fully handled by
the library. This is similar to programming with async / await in Javascript or
Python, goroutines in Go, or many similar systems. It's a much easier way to
achieve the same results as when using e.g. libuv or ASIO directly with
callbacks.

Since fibers are cooperatively scheduled, it's important to not block the whole
thread by calling existing synchronous API:s that might block for long or keep
the thread occupied for long without yielding control to the fiber scheduler. If
a blocking call (which is unaware of fibers) is unavoidable, it can be run on a
separate thread and fibers can easily wait for the result using a future (from
Boost.Fiber).

See
[Boost.Fiber's](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html)
documentation for more details. FiberIO support everything except changing the
scheduler, since FiberIO is a Boost.Fiber scheduler.


Documentation
-------------

Doxygen documentation is available
[here](https://hampus.github.io/fiberio/html/).


Advantages and features
-----------------------

These are some features and advantages:
* Natural programming model with full support for exceptions.
* Each fiber has its own stack. Only touched memory pages will typically get
  mapped by the OS.
* Cooperatively scheduled in user space, which avoids overhead associated with
  the kernel thread scheduler.
* Avoids allocating a kernel stack for each fiber, which is needed for threads.
* It's possible to create many more fibers than threads on most systems.
* Inherits most of the advantages (and disadvantages) of event-based I/O.

There are also some disadvantages:
* Cooperatively scheduled. No preemptive scheduling.
* Can't use existing libraries or API:s that use blocking I/O (unless they
  explicitly use this library for all their blocking calls).
* Allocates a stack for each fiber. This may add some memory overhead.


Requirements
------------

The library is built using [Meson](http://mesonbuild.com/) and depends on [Boost.Fiber](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html),
[libuv](http://libuv.org/) and (optionally)
[Google Test](https://github.com/google/googletest). It also requires C++14. If
C++17 is available, it will make use of it (specify -Dcpp_std=c++17 to meson).

On Ubuntu 18.04, you can install dependencies with:

    $ sudo apt install wget build-essential automake libtool pkg-config meson \
        libboost-fiber-dev libgtest-dev

You also need libuv, but the version in Ubuntu 18.04 is too old so you'll need
to build and install it manually from source.

On Arch Linux, you can install everything using pacman:

    $ sudo pacman -S  pkg-config meson boost libuv gtest

It's a good idea to also install ccache if rebuilding often.


Building
--------

Use Meson:

    $ meson -Dcpp_std=c++17 -Dbuildtype=release build
    $ cd build
    $ ninja test

This builds the library and some example applications under build/examples/. It
will also run all the unit tests. You can leave out cpp_std if you don't have
a C++17 compatible compiler.


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


Test Coverage
-------------

To get a coverage report, install lcov and build like this:

    $ meson -Db_coverage=true build
    $ cd build
    $ ninja clean && ninja test && ninja coverage

Simply repeat the last command to rebuild from scratch.

There's also a script for generating coverage reports directly with lcov. To
use it, change the last command to this instead:

    $ ninja clean && ninja test && ../lcov.sh

The lcov script generates better reports, since it won't include the example
applications in the coverage. There are intentionally no tests for those.


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
        fibers::async([](fiberio::socket client) {
            char buf[4096];
            while (client.is_open()) {
                std::size_t bytes_read{ client.read(buf, sizeof(buf)) };
                client.write(buf, bytes_read);
            }
        }, server.accept());
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

There's also a standard C++ iostream wrapper for sockets. For example:

```c++
fiberio::socket_stream stream{ socket };
stream << 3000;
stream.close();
```

An std::basic_iostream can also be created on the server side, e.g.:
```c++
fiberio::socket_stream stream{ server.accept() };
int number;
stream >> number;
```
