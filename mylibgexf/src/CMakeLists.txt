file( GLOB CPPS "*.cpp")
set (SRCS ${CPPS})

include_directories(
${GAT_SOURCE_DIR}/include
${GAT_SOURCE_DIR}/external/install/include
${LIBGEXF_TLDIR}/include
${LIBXML2_INCLUDE_DIR}
)

link_directories(
${GAT_SOURCE_DIR}/external/install/lib
${LIBXML2_LIBRARY_DIRS}
)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

#add_executable( ComputeSubgraphSignatures ${SUBGRAPH_SIG_SRCS} )

add_library(gexf SHARED ${SRCS})

add_custom_command(TARGET gexf POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libgexf.${SHARED_LIB_EXT} 
                   ${GAT_SOURCE_DIR}/external/install/lib/libgexf.${SHARED_LIB_EXT})


#install(TARGETS gexf DESTINATION ${GAT_SOURCE_DIR}/external/install/lib)
