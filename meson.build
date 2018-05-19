project('fiberio', 'cpp', default_options : ['cpp_std=c++14'])

if get_option('valgrind')
  message('Adding BOOST_USE_VALGRIND. Requires a compatible Boost build.')
  add_global_arguments('-DBOOST_USE_VALGRIND', language : 'cpp')
endif

thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['context', 'fiber'])
libuv_dep = dependency('libuv')

fiberio_inc = include_directories('include')

subdir('src')
subdir('tests')
subdir('examples')
