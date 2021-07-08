cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)

project(ecs CXX)

set(
    sources
    src/main.cpp
)

set(
    bench_src
    src/benchmark.cpp
)

# use dynamic wx
# add_compile_definitions(WXUSINGDLL)

# use static wx
#set(wxBUILD_SHARED OFF CACHE BOOL "use static wx" FORCE)

# add_subdirectory(vendor/wxWidgets)
# add_executable(wxfractal WIN32 ${sources})

# if(MSVC)
# 	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
# endif()

add_executable(demo ${sources})
add_executable(benchmark ${bench_src})
# target_link_libraries(wxfractal wx::core wx::base wx::net)

# set_property(TARGET wxfractal PROPERTY
#   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# set_property(TARGET wxfractal PROPERTY
#   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")