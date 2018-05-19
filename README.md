FiberIO - fiber-based C++ network library
=========================================

[![Travis](https://img.shields.io/travis/hampus/fiberio.svg)](https://travis-ci.org/hampus/fiberio)
[![license](https://img.shields.io/github/license/hampus/fiberio.svg)](http://www.apache.org/licenses/LICENSE-2.0)

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


Description
-------

FiberIO is a Boost.Fiber scheduler that supports running fibers on a single
thread and provides API:s for doing blocking I/O that integrates with the fiber
scheduler and is handled using libuv under the hood. This means that inter-fiber
synchronization is not necessary in many cases due to cooperative scheduling and
the lack of multi-threading. A function that does no I/O and doesn't call any
other functions that might yield control will always run to completion without
any fiber switches and can make use of that fact. In many ways this is similar
to programming with async / await in Javascript or Python, promises and futures,
NodeJS, or even explicit event-based programming using libuv, libevent, epoll,
kqueue, or IOCP on a single thread. It's just easier and more natural.

At the same time, it also looks a lot like writing traditional threaded source
code with blocking I/O calls. It's also possible to freely call libraries that
are unaware of fibers and threads and even make blocking FiberIO calls in
callbacks from any such libraries. The main potential problem is that if
something blocks the thread that the fibers run on, no other fiber will be able
to run. Blocking calls should only be made through this library (or Boost.Fiber)
so that they block only the fiber and yield control to the event loop behind the
scenes. This is the same limitation that explicit asynchronous I/O on a single
thread also has.

When multiple CPU cores are needed or you want to use e.g. memory mapped file
access, using ordinary threads is generally a better idea.

See
[Boost.Fiber's](https://www.boost.org/doc/libs/release/libs/fiber/doc/html/index.html)
documentation for details. It's not yet possible to communicate between fibers
and other threads, but that will be supported in the future.


Documentation
-------------

Doxygen documentation is available
[here](https://hampus.github.io/fiberio/html/).


Threads vs Fibers vs Async
--------------------------

This compares fibers of the FiberIO kind to threads and classic asynchronous
programming.

These are some advantages of fibers:
* More natural way to program than using promises/futures, async/await or
  callbacks. Full support for exceptions and ordinary function calls.
* Avoids the OS thread scheduler when switching between fibers and can
  potentially use a simple scheduler for fast context switches.
* Possible to run cooperatively on a single thread with blocking I/O (that
  yields control) and minimal thread synchronization.
* Less risk for mysterious thread synchronization bugs when run on one thread.
* Cooperatively scheduled. Predictable (in a way) and can reduce overhead.
* Using a stack for memory allocations is very fast (same for threads).

Fibers also have these disadvantages:
* Can't use existing libraries or API:s that use blocking I/O or similar (unless
  they explicitly use this library for all their blocking calls).
* There's no easy way to tell if a function call is "safe" or might block the
  whole thread rather than just the fiber itself.
* Page faults block the whole thread and won't suspend only the fiber itself
  (makes memory-mapped I/O impossible).
* Running fibers on multiple threads basically requires the same kind of
  scheduling and inter-fiber synchronization as threads.
* The OS can't suspend or wake up individual fibers directly in any way.
* Cooperatively scheduled. Preemptive scheduling can be useful.
* Needs to allocate a stack for each fiber, in contrast to event-based systems.

In many ways, fibers are a mix between classic threads and classic asynchronous
programming, where fibers look almost like threads but still aren't.

A typical scenario where fibers shine is when they are run on a single thread
and I/O is the main bottleneck. Event-based I/O is often used in these
situations, like e.g. NodeJS, libuv, libevent, Python's asyncio, Redis, nginx,
libwayland, Folly, Finagle, Boost.Asio with callbacks and so on. Fibers behave
the same way as these systems and offer an easier programming model. Note that
some of these systems can make use of more than one thread, but they are still
fundamentally event-based and not thread-based.


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
    $ ninja test && ninja coverage


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
