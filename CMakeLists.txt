cmake_minimum_required (VERSION 2.6)

project (GHOST)

#set (CMAKE_CXX_COMPILER g++)

set (CMAKE_CXX_FLAGS "-std=c++11 -O3")# -Wl,-rpath,\$$ORIGIN/../lib") 

set (GAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_VERBOSE_MAKEFILE true)

find_package(Boost 1.50.0 COMPONENTS iostreams filesystem system program_options )

add_subdirectory (3rdParty)

add_subdirectory (src)

