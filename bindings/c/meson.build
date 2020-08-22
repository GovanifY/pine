project('pcsx2_ipc', 'cpp', default_options : ['cpp_std=c++17'])
add_global_arguments('-DC_FFI', language : 'cpp')


incdir = include_directories('../../')
winsock = dependency('ws2_32', required : false)
thread_dep = dependency('threads')
src = ['c_ffi.cpp', 'c_ffi.h', '../../pcsx2_ipc.h']
library('pcsx2_ipc_c', src, dependencies : [thread_dep, winsock],
include_directories: incdir)

cleaner = find_program('clang-format')
r = run_command(cleaner, '-i', src)
