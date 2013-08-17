#ifndef SIGNATURE_MAP_HPP
#define SIGNATURE_MAP_HPP

#include <iostream>
#include <unordered_map>

#include "GraphUtils.hpp"

using std::cerr;
using std::string;
using std::unique_ptr;
using std::unordered_map;

using GraphUtils::Graph;

class SignatureMap {
	typedef boost::graph_traits<GraphUtils::Graph>::vertex_descriptor Vertex;

public:
	explicit SignatureMap(const std::string& sigFile, Graph& G) : G_(G) {
		cerr << "reading signature file from : " << sigFile << "\n";
		unordered_map<string, Vertex> nameToIDG;
		GraphUtils::Graph::vertex_iterator b,e;
		std::tie(b,e) = boost::vertices(G);
		for ( auto v = b; v < e; ++v ) { nameVertexMap_[(G)[*v].name] = *v; }

	}

private:
	Graph& G_;
	unordered_map<string, Vertex> nameVertexMap_;
};


#endif // SIGNATURE_MAP_HPP