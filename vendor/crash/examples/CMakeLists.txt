cmake_minimum_required(VERSION 3.10)

add_executable(crash-demo "demo.c")
add_executable(crash::demo ALIAS crash-demo)
set(LIBCRASH_SHARED_LIBS "ON")
target_link_libraries(crash-demo crash::crash)