CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT (GHOST)

ENABLE_TESTING()

SET(CPACK_PACKAGE_VERSION "0.5.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "5")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_PACKAGE_VENDOR "Carnegie Mellon University")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GHOST - Global Network Alignment using Multiscale Spectral Signatures")
SET(CPACK_PACKAGE_NAME 
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Source")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET (GAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

SET(CMAKE_VERBOSE_MAKEFILE ON)

SET (CMAKE_BUILD_TYPE Release)
SET (CMAKE_CXX_FLAGS "-O3 -DHAVE_ANSI_TERM -DNDEBUG -std=c++11 -Wno-deprecated ${CMAKE_CXX_FLAGS}")


FIND_PACKAGE (Boost 1.53 COMPONENTS iostreams filesystem system program_options )


########
##
#   Fetching of external dependencies
##
########
INCLUDE (ExternalProject)


##
# Either inform the user of how to obtain Boost, or, if they passed in the FETCH_BOOST
# option, go and grab it for them.
##
if ((NOT Boost_FOUND) AND (NOT FETCH_BOOST))
	message(FATAL_ERROR 
		"GHOST cannot be compiled without Boost.\n"
        "It is recommended to visit http://www.boost.org/ and install Boost according to those instructions.\n"
        "This build system can also download and install a local version of boost for you (this can take a *lot* of time).\n"
        "To fetch and build boost locally, call cmake with -DFETCH_BOOST=TRUE"
    )
elseif(FETCH_BOOST)
    ## Let the rest of the build process know we're going to be fetching boost
    set(BOOST_WILL_RECONFIGURE TRUE)
    message("Build system will fetch and build Boost")
    message("==================================================================")
    ExternalProject_Add(libboost
        DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
        URL http://www.cs.cmu.edu/~robp/files/boost-1.53.0.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/boost_1_53_0
        INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
        CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/boost_1_53_0/bootstrap.sh ${BOOST_CONFIGURE_TOOLSET} --prefix=<INSTALL_DIR>
        BUILD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/external/boost_1_53_0/b2 ${BOOST_EXTRA_FLAGS} install
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
    )

    ##
    # After we've installed boost, 
    ##
    ExternalProject_Add_Step(libboost reconfigure
        COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR} -DBOOST_WILL_RECONFUGRE=FALSE -DBOOST_RECONFIGURE=TRUE 
        DEPENDEES install
    )
endif()

##
# If we're fetching boost and we need to have dummy paths for these variables
# so that CMake won't complain
##
if (BOOST_WILL_RECONFIGURE)
    message("Setting Temporary Boost paths")
    set(Boost_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install/include)
    set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/include)
    set(Boost_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib)        
    set(Boost_FOUND TRUE)
endif()

##
#  If we had to fetch Boost, the reconfigure step will re-run cmake.  The second configuration
#  pass is executed with the BOOST_RECONFIGURE flag set. This should allow our newly
#  installed Boost to be found by CMake.
##
if (BOOST_RECONFIGURE)
    unset(Boost_FOUND CACHE)
    unset(Boost_INCLUDE_DIR CACHE)
    unset(Boost_INCLUDE_DIRS CACHE)
    unset(Boost_LIBRARY_DIRS CACHE) 
    unset(Boost_LIBRARIES CACHE)
    unset(BOOST_ROOT CACHE)
    unset(CMAKE_PREFIX_PATH CACHE)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/install)
    set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/include)
    set(Boost_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib)
    find_package(Boost 1.53.0 COMPONENTS iostreams filesystem system thread timer program_options serialization)
endif()


message("BOOST INCLUDE DIR = ${Boost_INCLUDE_DIR}")
message("BOOST INCLUDE DIRS = ${Boost_INCLUDE_DIRS}")
message("BOOST LIB DIR = ${Boost_LIBRARY_DIRS}")
message("BOOST LIBRAREIS = ${Boost_LIBRARIES}")
find_package (ZLIB)

set(EXTERNAL_LIBRARY_PATH $CMAKE_CURRENT_SOURCE_DIR/lib)


###
#  Fetch and build libxml2 if necessary
###
find_package(LibXml2)

if(NOT LIBXML2_FOUND)

message("Build system will fetch and build LibXML2")
message("==================================================================")

set(LIBXML2_WILL_RECONFIGURE TRUE)
set(LIBXML2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/libxml2)
set(LIBXML2_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install)

ExternalProject_Add(libxml2
	DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
	URL ftp://xmlsoft.org/libxml2/libxml2-2.9.1.tar.gz
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/libxml2
	INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --without-python --prefix=${LIBXML2_INSTALL_DIR}
	BUILD_COMMAND make
	INSTALL_COMMAND make install
)

ExternalProject_Add_Step(libxml2 reconfigure
  COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR} -DLIBXML2_WILL_RECONFIGURE=FALSE -DLIBXML2_RECONFIGURE=TRUE
  DEPENDEES install
)
endif()

##
# If we're fetching tbb, we need to have dummy paths for these variables
# so that CMake won't complain
##
if(LIBXML2_WILL_RECONFIGURE)
    set(LIBXML2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install/include/libxml2)
    set(LIBXML2_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib)
    set(LIBXML2_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib/libxml2.${SHARED_LIB_EXT})
endif()

##
#  Similar to the Boost trick above, the libtbb reconfigure should force this code
#  to be run on the second configuration pass, where it should appropriately set the
#  TBB_INSTALL_DIR variable.
##
if (LIBXML2_RECONFIGURE)
    unset(LIBXML2_FOUND CACHE)
    unset(LIBXML2_INSTALL_DIR CACHE)
    unset(CMAKE_PREFIX_PATH CACHE)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/install)
    set(LIBXML2_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install)
    message("LIBXML2_INSTALL_DIR = ${LIBXML2_INSTALL_DIR}")
    find_package(LibXml2)
endif()


message("TBB_LIBRARIES = ${TBB_LIBRARIES}")
##
#
# Fetch and build Intel's Threading Building Blocks library.
#
##
find_package(TBB)
if(NOT TBB_FOUND)

set(TBB_WILL_RECONFIGURE TRUE)
# Set the appropriate compiler
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

    set(TBB_COMPILER "clang")
    ##
    # On OSX, tbb currently has a linker bug so we have to patch up the following file
    ##
   if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    set(TBB_LINK_SYMBOLS "${CMAKE_CURRENT_SOURCE_DIR}/external/tbb41_20130314oss/src/tbb/mac64-tbb-export.lst")
    set(TBB_PATCH_STEP awk "!/__TBB_SYMBOL\( _ZTVN3tbb13tbb_exceptionE \)/" ${TBB_LINK_SYMBOLS} > ${CMAKE_CURRENT_SOURCE_DIR}/external/tbb41_20130314oss/tmplnk && mv ${CMAKE_CURRENT_SOURCE_DIR}/external/tbb41_20130314oss/tmplnk ${TBB_LINK_SYMBOLS})
   else("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    set(TBB_PATCH_STEP "")
   endif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")

else()

    set(TBB_COMPILER "gcc")

endif()

message("Build system will fetch and build Intel Threading Building Blocks")
message("==================================================================")
# These are useful for the custom install step we'll do later
set(TBB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tbb41_20130314oss)
set(TBB_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install)

ExternalProject_Add(libtbb
	DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
	URL http://threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb41_20130314oss_src.tgz
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tbb41_20130314oss
    INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
    PATCH_COMMAND "${TBB_PATCH_STEP}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make compiler=${TBB_COMPILER} cfg=release tbb_build_prefix=LIBS
    INSTALL_COMMAND sh -c "mkdir -p ${TBB_INSTALL_DIR}/lib && mkdir -p ${TBB_INSTALL_DIR}/include && cp ${TBB_SOURCE_DIR}/build/LIBS_release/*.${SHARED_LIB_EXT}* ${TBB_INSTALL_DIR}/lib && cp -r ${TBB_SOURCE_DIR}/include/* ${TBB_INSTALL_DIR}/include"
    BUILD_IN_SOURCE 1
)

ExternalProject_Add_Step(libtbb reconfigure
        COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR} -DTBB_WILL_RECONFIGURE=FALSE -DTBB_RECONFIGURE=TRUE
        DEPENDEES install
)
endif()

##
# If we're fetching tbb, we need to have dummy paths for these variables
# so that CMake won't complain
##
if(TBB_WILL_RECONFIGURE)
    set(TBB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/include)
    set(TBB_LIBRARY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib)
    set(TBB_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib/libtbb.${SHARED_LIB_EXTE}
                      ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib/libtbbmalloc.${SHARED_LIB_EXT}
    )
endif()

##
#  Similar to the Boost trick above, the libtbb reconfigure should force this code
#  to be run on the second configuration pass, where it should appropriately set the
#  TBB_INSTALL_DIR variable.
##
if (TBB_RECONFIGURE)
    unset(TBB_FOUND CACHE)
    unset(TBB_INSTALL_DIR CACHE)
    unset(CMAKE_PREFIX_PATH CACHE)
    set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/install)
    set(TBB_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install)
    message("TBB_INSTALL_DIR = ${TBB_INSTALL_DIR}")
    find_package(TBB)
endif()


message("TBB_LIBRARIES = ${TBB_LIBRARIES}")

########
##
#   END: Fetching of external dependencies
##
########


ADD_SUBDIRECTORY (3rdParty)

set(ORIG_ARCHIVE_DIR ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/install/lib)

ADD_SUBDIRECTORY (mylibgexf)
ADD_SUBDIRECTORY (src)


SET(CPACK_SET_DESTDIR ON)
SET(CPACK_BUNDLE_NAME "GHOST")
SET(CPACK_BUNDLE_IDENTIFIER "edu.cmu.cs.ghost")
SET(CPACK_PACKAGING_INSTALL_PREFIX ".")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "${CPACK_PACKAGE_NAME}")
SET(CPACK_PACKAGE_EXECUTABLES "ComputeSubgraphSignatures" "ghost")
SET(CPACK_PACKAGE_NAME "GHOST")
SET(CPACK_PACKAGE_VENDOR "Robert Patro & Carl Kingsford; Carnegie Mellon University")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Global Network Alignment Using Multiscale Spectral Signatures")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "5")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-src")
SET(CPACK_SOURCE_IGNORE_FILES
  "/doc/;/.git/;.gitignore;/build/;/.bzr/;~$;${CPACK_SOURCE_IGNORE_FILES}")
INCLUDE( CPack )




