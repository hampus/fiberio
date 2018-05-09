project('fiberio', 'cpp', default_options : ['cpp_std=c++14'])

thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['context', 'fiber'])
libuv_dep = dependency('libuv')

fiberio_inc = include_directories('include')

subdir('src')
subdir('tests')
subdir('example')
