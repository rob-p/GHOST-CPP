set (SUBGRAPH_SIG_SRCS ComputeSubgraphSignatures.cpp util.cpp )
set (GHOST_SRCS Ghost.cpp GlobalAlignmentDriver.cpp util.cpp )

include_directories( 
${GAT_SOURCE_DIR}/include
${GAT_SOURCE_DIR}/external/install/include
${GAT_SOURCE_DIR}/3rdParty/g2log/src
${LIBXML2_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
${EIGEN3_INCLUDE_DIR}
)

link_directories( 
${GAT_SOURCE_DIR}/external/install/lib
${CMAKE_CURRENT_BINARY_DIR}/3rdParty/g2log
${TBB_LIBRARY_DIRS}
)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable( ComputeSubgraphSignatures ${SUBGRAPH_SIG_SRCS} )
add_executable( Ghost ${GHOST_SRCS} )
target_link_libraries( ComputeSubgraphSignatures
${DYN_LINK_CMD}
${LIBXML2_LIBRARIES}
gexf
${TBB_LIBRARIES}
${Boost_LIBRARIES}
lib_activeobject
lib_g2logger
)

target_link_libraries( Ghost 
${DYN_LINK_CMD}
${LIBXML2_LIBRARIES}
gexf
${TBB_LIBRARIES}
${Boost_LIBRARIES}
lib_activeobject
lib_g2logger
)


add_dependencies(ComputeSubgraphSignatures libeigen)
add_dependencies(ComputeSubgraphSignatures libboost)
add_dependencies(ComputeSubgraphSignatures libtbb)
add_dependencies(ComputeSubgraphSignatures libxml2)
add_dependencies(Ghost libeigen)
add_dependencies(Ghost libboost)
add_dependencies(Ghost libtbb)
add_dependencies(Ghost libxml2)
