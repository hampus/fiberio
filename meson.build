project('fiberio', 'cpp', default_options : ['cpp_std=c++14'])

thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['context', 'fiber'])
libuv_dep = dependency('libuv')

libfiberio_srcs = [
    'src/fiberio.cpp',
    'src/tcpsocket.cpp',
    'src/addrinfo.cpp',
    'src/detail/scheduler.cpp',
    'src/detail/utils.cpp'
]

libfiberio_inc = include_directories('include')
libfiberio_deps = [thread_dep, boost_dep, libuv_dep]

libfiberio = library('fiberio', libfiberio_srcs,
    include_directories : libfiberio_inc,
    dependencies : libfiberio_deps)

executable('fibertest', ['example/fibertest.cpp'],
    include_directories : libfiberio_inc,
    dependencies : libfiberio_deps,
    link_with : libfiberio)
