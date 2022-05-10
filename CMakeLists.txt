cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)
project(compos CXX)

# Library
add_library(compos INTERFACE)
target_include_directories(compos INTERFACE include)
# target_sources(compos INTERFACE include/compos.h)

# Vendor dependencies
add_subdirectory(vendor/fmt EXCLUDE_FROM_ALL)
# set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(LIBCRASH_SHARED_LIBS "YES")
add_subdirectory(vendor/crash EXCLUDE_FROM_ALL)

# Binary targets
set(demo_src src/main.cpp)
set(bench_src src/benchmark.cpp)

add_executable(demo ${demo_src})
target_link_libraries(demo compos fmt::fmt)


add_executable(benchmark ${bench_src})
target_link_libraries(benchmark compos)
target_link_libraries(benchmark fmt::fmt)
target_link_libraries(benchmark crash::crash)

function(target_enable_warnings target scope)
    target_compile_options(
        ${target} ${scope}
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
    )
endfunction()

target_enable_warnings(benchmark PRIVATE)
