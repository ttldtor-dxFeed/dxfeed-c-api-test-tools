cmake_minimum_required(VERSION 3.8.0)

cmake_policy(SET CMP0015 NEW)

project(dxfeed-c-api-test-tools)

set(TARGET_PLATFORM "x64" CACHE STRING "Target platform specification")
set(DISABLE_TLS on CACHE BOOL "Build without the TLS support")

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(TARGET_PLATFORM "x86")
endif ()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(dxfeed-c-api/include dxfeed-c-api/wrappers/cpp/thirdparty/fmt-8.0.0/include thirdparty/boost-1_75)
include_directories(dxfeed-cxx-api)

add_definitions(-DFMT_HEADER_ONLY=1)

add_subdirectory(c-api-lib)
add_subdirectory(tools/mt-reader)
add_subdirectory(tools/collision-detector)
add_subdirectory(tools/plb-tester)
add_subdirectory(tools/bench)

