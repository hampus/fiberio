
gtest_dep = dependency('gtest', main : true, required : false)

if not gtest_dep.found()
  subdir_done()
endif

test_deps = [fiberio_dep, gtest_dep]

server_socket = executable('server_socket', 'server_socket_tests.cpp',
  dependencies : test_deps)
test('server_socket', server_socket)

scheduling = executable('scheduling', 'scheduling_tests.cpp',
  dependencies : test_deps)
test('scheduling', scheduling)
