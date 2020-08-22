project('pcsx2_ipc_c', 'cpp', default_options : ['cpp_std=c++17'])

winsock = dependency('ws2_32', required : false)
thread_dep = dependency('threads')
src = ['src/client.cpp', 'src/pcsx2_ipc.h']
executable('client', src, dependencies : [thread_dep, winsock])

cleaner = find_program('clang-format')
r = run_command(cleaner, '-i', src)
