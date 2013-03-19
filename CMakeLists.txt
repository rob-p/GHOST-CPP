cmake_minimum_required (VERSION 2.6)

project (GHOST)

set (CMAKE_CXX_COMPILER g++)

set (CMAKE_CXX_FLAGS "-std=c++11")

set (GAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_VERBOSE_MAKEFILE true)

add_subdirectory ( src )
