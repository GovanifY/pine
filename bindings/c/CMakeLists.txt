cmake_minimum_required(VERSION 3.10)
project(pine_c LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(pine_c SHARED c_ffi.cpp)
target_compile_definitions(pine_c PRIVATE C_FFI)
target_include_directories(pine_c
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(pine_c PRIVATE Threads::Threads)
if(WIN32)
  target_link_libraries(pine_c PRIVATE ws2_32)
endif()
