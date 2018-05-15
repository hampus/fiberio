project('fiberio', 'cpp', default_options : ['cpp_std=c++17'])

conf_data = configuration_data()

if get_option('valgrind')
  message('Adding BOOST_USE_VALGRIND. Requires a compatible Boost build.')
  add_global_arguments('-DBOOST_USE_VALGRIND', language : 'cpp')
endif

iobackend = get_option('iobackend')
use_epoll = false
use_libuv = false
if iobackend == 'epoll'
  message('I/O backend: epoll')
  use_epoll = true
elif iobackend == 'libuv'
  message('I/O backend: libuv')
  use_libuv = true
else
  message('I/O backend: libuv')
  use_libuv = true
endif

thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['context', 'fiber'])
fiberio_deps = [thread_dep, boost_dep]

if use_libuv
  libuv_dep = dependency('libuv')
  fiberio_deps += libuv_dep
endif

fiberio_inc = include_directories('include', '.')

conf_data.set('USE_EPOLL', use_epoll)
conf_data.set('USE_LIBUV', use_libuv)
configure_file(
  input : 'config.hpp.in',
  output : 'config.hpp',
  configuration : conf_data)

subdir('src')
subdir('tests')
subdir('examples')
