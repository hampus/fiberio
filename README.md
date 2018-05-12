FiberIO - fiber-based C++ network library
=========================================

This is an experimental C++ network library built on top of Boost.Fiber.

See [NOTICE](NOTICE) and [LICENSE](LICENSE) for license terms.

This is highly experimental at this point. Currently uses libuv internally for
asynchronous networking.


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
