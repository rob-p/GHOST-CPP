cmake_minimum_required (VERSION 2.8)

project (libgexf)

set (CMAKE_CXX_COMPILER g++)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   MESSAGE ("-- Detected OSX")   
   SET (OperatingSystem "Mac OS X")
   SET (SHARED_LIB_EXT "dylib")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   MESSAGE ("-- Detected Linux")
   SET (OperatingSystem "Linux")  
   SET (SHARED_LIB_EXT "so")
ELSE ()
   MESSAGE ("WHAT's THE OS? ${CMAKE_SYSTEM_NAME}")
ENDIF()

if (${OperatingSystem} MATCHES "Mac OS X")
	set (CMAKE_CXX_FLAGS "-undefined dynamic_lookup")
elseif(${OperatingSystem} MATCHES "Linux")
	set (CMAKE_CXX_FLAGS "-Wl,--no-undefined")
endif()

set (LIBGEXF_TLDIR ${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_VERBOSE_MAKEFILE true)

find_package(LibXml2)

if (NOT LIBXML2_FOUND)
	message (status "COULDN'T FIND LIBXML2")
endif()

add_subdirectory ( src )
