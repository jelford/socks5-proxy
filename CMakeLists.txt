cmake_minimum_required (VERSION 2.6)
project (socks-proxy)

set (socks-proxy_VERSION_MAJOR 1)
set (socks-proxy_VERSION_MINOR 0)

# Set the compiler to use C++11
add_definitions("-std=c++11")
add_definitions("-Wall")
add_definitions("-pedantic")
add_definitions("-g")
add_definitions("-pg")

# Include the Sockets submodule
include_directories ("${PROJECT_SOURCE_DIR}"Sockets/include/)
add_subdirectory (Sockets)

#include_directories ("${PROJECT_SOURCE_DIR}"socks/include/)
#add_subdirectory (socks)

add_executable (socks-proxy main.cpp)
target_link_libraries (socks-proxy Sockets pthread)
