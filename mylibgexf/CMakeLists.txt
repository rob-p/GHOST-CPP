cmake_minimum_required (VERSION 2.6)

project (libgexf)

set (CMAKE_CXX_COMPILER g++)

set (GAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_VERBOSE_MAKEFILE true)

#find_package(LibXml2 REQUIRED)

add_subdirectory ( src )
