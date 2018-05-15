
fiberio_srcs = [
  'fiberio.cpp',
  'server_socket.cpp',
  'socket.cpp',
]

if use_libuv
  fiberio_srcs += [
    'uv/server_socket_impl.cpp',
    'uv/socket_impl.cpp',
    'uv/addrinfo.cpp',
    'uv/scheduler.cpp',
    'uv/loop.cpp',
    'uv/utils.cpp'
  ]
endif

libfiberio = library('fiberio', fiberio_srcs,
  include_directories : fiberio_inc,
  dependencies : fiberio_deps)

fiberio_dep = declare_dependency(
  include_directories : fiberio_inc,
  dependencies : fiberio_deps,
  link_with : libfiberio)
