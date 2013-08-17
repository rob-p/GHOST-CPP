#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <regex>
#include <tuple>
#include <cstdlib>
#include <memory>
#include <typeinfo>
#include <thread>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/graph_utility.hpp>

#include <libgexf/libgexf.h>

#include "g2logworker.h"
#include "g2log.h"

#include "tbb/concurrent_queue.h"
#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include "ezETAProgressBar.hpp"

using std::cout;
using std::string;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;

namespace GraphUtils {

  libgexf::GEXF read( const string& fname ) {
    libgexf::FileReader* reader = new libgexf::FileReader();
    reader->init(fname);
    reader->slurp();
    return reader->getGEXFCopy();
  }

  libgexf::t_id findAttributeID( const libgexf::Data& data, const string& aname ) {
    const auto& attr = data.getNodeAttributeColumn();
    while( attr->hasNext() ) {
      auto aid = attr->next();
      if ( attr->currentTitle() == aname )
        return aid;
    }
    return "";
  }


  struct Vertex{
       string name; // or whatever, maybe nothing
  };
  typedef boost::property<boost::vertex_index_t, size_t, Vertex> vertex_prop;

  struct Edge{
       // nothing right now
  };
  typedef boost::property<boost::edge_index_t, size_t, Edge> edge_prop;

  typedef boost::adjacency_list <
        boost::vecS,                //  The container used for egdes : here, std::vector.
        boost::vecS,                //  The container used for vertices: here, std::vector.
        boost::undirectedS,         //  undirected graph
        vertex_prop,                //  The type that describes a Vertex.
        edge_prop                   //  The type that describes an Edge
    > GraphT;

    typedef boost::subgraph<GraphT> Graph;
    typedef Graph::vertex_descriptor VertexID;
    typedef Graph::edge_descriptor   EdgeID;

  unique_ptr<Graph> readGraphFromGEXF(const std::string& fname) {

    auto GContainer = read(fname);
    auto IG = GContainer.getUndirectedGraph();
    auto GData = GContainer.getData();

    unique_ptr<Graph> G(new Graph(IG.getNodeCount()));

    auto attributeID = findAttributeID(GData, "gname");

    std::map<libgexf::t_id, VertexID> nodeNodeMap;

    const auto& nit = IG.getNodes()->begin();
    size_t nodeCnt = 0;
    while ( nit->hasNext() ) {
      auto nid = nit->next();
      auto name = GData.getNodeAttribute(nid, attributeID);
      LOG(INFO) << "node : " << name << "\n";
      nodeNodeMap[nid] = nodeCnt;
      (*G)[nodeCnt].name = name;
      ++nodeCnt;
    }

    const auto& eit = IG.getEdges()->begin();
    while ( eit->hasNext() ) {
      eit->next();
      auto s = nodeNodeMap[eit->currentSource()];
      auto t = nodeNodeMap[eit->currentTarget()];
      boost::add_edge(s,t,*G);
    }

    Graph::vertex_iterator b,e;
    std::tie(b,e) = boost::vertices(*G);
    for ( auto v = b; v < e; ++v ) {
      LOG(INFO) << "node : " << (*G)[*v].name << " -> " << *v << "\n";
    }

    return G;
  }

}

#endif // GRAPH_UTILS