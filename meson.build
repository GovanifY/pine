project('pcsx2_ipc_c', 'cpp', default_options : ['cpp_std=c++17',
  'buildtype=release'])

winsock = dependency('ws2_32', required : false)
thread_dep = dependency('threads')
src = ['src/client.cpp', 'src/pcsx2_ipc.h']
executable('client', src, dependencies : [thread_dep, winsock])

test_src = ['src/tests.cpp']
e = executable('tests', test_src)
test('tests', e)


cleaner = find_program('clang-format', required : false)
if cleaner.found()
  run_command(cleaner, '-i', src)
  run_command(cleaner, '-i', test_src)
endif
