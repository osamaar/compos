cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
project(libcrach C)

# Library
if (DEFINED LIBCRASH_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${LIBCRASH_SHARED_LIBS}")
endif ()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(crash)
add_library(crash::crash ALIAS crash)
target_sources(crash PUBLIC "src/crash.c" "include/crash.h")
target_include_directories(crash PUBLIC include)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(examples EXCLUDE_FROM_ALL)