set (SUBGRAPH_SIG_SRCS ComputeSubgraphSignatures.cpp util.cpp )
set (GHOST_SRCS Ghost.cpp GlobalAlignmentDriver.cpp util.cpp )

include_directories( 
${GAT_SOURCE_DIR}/include
${GAT_SOURCE_DIR}/external/install/include
${GAT_SOURCE_DIR}/3rdParty/g2log/src
${LIBXML2_INCLUDE_DIR}
/opt/local/include/eigen3
/fs/szattic-asmg6/rob/dist/stow/eigen-3.1/include/
/home/robp/dist/stow/eigen-3.1.2/include
)

link_directories( 
${GAT_SOURCE_DIR}/lib 
${GAT_SOURCE_DIR}/external/install/lib
${CMAKE_CURRENT_BINARY_DIR}/3rdParty/g2log
)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable( ComputeSubgraphSignatures ${SUBGRAPH_SIG_SRCS} )
add_executable( Ghost ${GHOST_SRCS} )
target_link_libraries( ComputeSubgraphSignatures xml2 
"-Wl,-Bdynamic" gexf
tbb
${Boost_LIBRARIES}
lib_activeobject
lib_g2logger
)

target_link_libraries( Ghost xml2 gexf
tbb
${Boost_LIBRARIES}
lib_activeobject
lib_g2logger
)


