
fiberio_deps = [thread_dep, boost_dep, libuv_dep]

fiberio_srcs = [
  'fiberio.cpp',
  'server_socket.cpp',
  'server_socket_impl.cpp',
  'socket.cpp',
  'socket_impl.cpp',
  'addrinfo.cpp',
  'scheduler.cpp',
  'loop.cpp',
  'utils.cpp'
]

libfiberio = library('fiberio', fiberio_srcs,
  include_directories : fiberio_inc,
  dependencies : fiberio_deps)

fiberio_dep = declare_dependency(
  include_directories : fiberio_inc,
  dependencies : fiberio_deps,
  link_with : libfiberio)
