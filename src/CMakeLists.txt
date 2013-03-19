set (SUBGRAPH_SIG_SRCS ComputeSubgraphSignatures.cpp )

include_directories( 
${GAT_SOURCE_DIR}/include
/opt/local/include
/opt/local/include/libxml2
/opt/local/include/eigen3
/fs/szattic-asmg6/rob/dist/include
/fs/szattic-asmg6/rob/dist/include/libxml2
/fs/szattic-asmg6/rob/dist/stow/eigen-3.1/include/
)

link_directories( 
${GAT_SOURCE_DIR}/lib 
/opt/local/lib
/opt/local/lib/gcc47
/fs/szattic-asmg6/rob/dist/lib
/fs/szattic-asmg6/rob/dist/lib64
)

add_executable( ComputeSubgraphSignatures ${SUBGRAPH_SIG_SRCS} )
target_link_libraries( ComputeSubgraphSignatures xml2 gexf
tbb
redsvd)
