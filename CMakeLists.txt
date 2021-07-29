cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)
project(ecs CXX)

# Library
add_library(ecs INTERFACE)
target_include_directories(ecs INTERFACE include)
# target_sources(ecs INTERFACE include/ecs.h)

# Vendor dependencies
add_subdirectory(vendor/fmt EXCLUDE_FROM_ALL)
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(LIBCRASH_SHARED_LIBS "YES")
add_subdirectory(vendor/crash EXCLUDE_FROM_ALL)

# Binary targets
set(demo_src src/main.cpp)
set(bench_src src/benchmark.cpp)

add_executable(demo ${demo_src})
target_link_libraries(demo ecs fmt::fmt)


add_executable(benchmark ${bench_src})
target_link_libraries(benchmark ecs)
target_link_libraries(benchmark fmt::fmt)
target_link_libraries(benchmark crash::crash)