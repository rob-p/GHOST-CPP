set (SUBGRAPH_SIG_SRCS ComputeSubgraphSignatures.cpp util.cpp )
set (GHOST_SRCS Ghost.cpp GlobalAlignmentDriver.cpp util.cpp )

include_directories( 
${GAT_SOURCE_DIR}/include
${GAT_SOURCE_DIR}/mylibgexf/include
${GAT_SOURCE_DIR}/3rdParty/g2log/src
/opt/local/include
/opt/local/include/libxml2
/opt/local/include/eigen3
/fs/szattic-asmg6/rob/dist/include
/fs/szattic-asmg6/rob/dist/include/libxml2
/fs/szattic-asmg6/rob/dist/stow/eigen-3.1/include/
/home/robp/dist/include
/home/robp/dist/include/libgexf-0.1.1
/home/robp/dist/include/libxml2
/home/robp/dist/stow/eigen-3.1.2/include
)

link_directories( 
${GAT_SOURCE_DIR}/lib 
/opt/local/lib
/fs/szattic-asmg6/rob/dist/lib
/fs/szattic-asmg6/rob/dist/lib64
/home/robp/dist/lib
${CMAKE_CURRENT_BINARY_DIR}/3rdParty/g2log
)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable( ComputeSubgraphSignatures ${SUBGRAPH_SIG_SRCS} )
add_executable( Ghost ${GHOST_SRCS} )
target_link_libraries( ComputeSubgraphSignatures xml2 gexf
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


