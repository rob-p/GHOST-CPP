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

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <libgexf/libgexf.h>

#include <lemon/smart_graph.h>
#include <lemon/concepts/graph.h>

#include "tclap/CmdLine.h"

#include "tbb/concurrent_queue.h"
#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include "actors/SubgraphExtractionActors.hpp"

//#include "cppa/cppa.hpp"

using std::cout;
using std::string;
using std::make_shared;
using std::shared_ptr;

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

int main(int argc, char* argv[]) {

  try {

    TCLAP::CmdLine cmd("Find unannotated proteins in an alignment", ' ', "1.0");
    TCLAP::ValueArg<string> graphName("g", "graph", "graph filename", true, "", "string");
    cmd.add(graphName);

    cmd.parse(argc, argv);

    auto GContainer = read(graphName.getValue());
    auto IG = GContainer.getUndirectedGraph();
    auto GData = GContainer.getData();

    struct Vertex{
       string name; // or whatever, maybe nothing
    };
    typedef boost::property<boost::vertex_index_t, size_t, Vertex> vertex_prop;

    struct Edge{
       // nothing right now
    };
    typedef boost::property<boost::edge_index_t, size_t, Edge> edge_prop;

    typedef boost::adjacency_list <
        boost::vecS,                //  The container used for egdes : here, std::list.
        boost::vecS,                //  The container used for vertices: here, std::vector.
        boost::undirectedS,         //  undirected graph
        vertex_prop,                     //  The type that describes a Vertex.
        edge_prop                        //  The type that describes an Edge
    > GraphT;

    typedef boost::subgraph<GraphT> Graph;
    
    typedef Graph::vertex_descriptor VertexID;
    typedef Graph::edge_descriptor   EdgeID;

    Graph G(IG.getNodeCount());

    auto attributeID = findAttributeID( GData, "gname" );

    
    std::map<libgexf::t_id, VertexID> nodeNodeMap;

    const auto& nit = IG.getNodes()->begin();
    size_t nodeCnt = 0;
    while ( nit->hasNext() ) {
      auto nid = nit->next();
      auto name = GData.getNodeAttribute(nid, attributeID);
      std::cerr << "node : " << name << "\n";
      nodeNodeMap[nid] = nodeCnt;
      G[nodeCnt].name = name;
      ++nodeCnt;
      //nodeList.push_back(u);
    }

    const auto& eit = IG.getEdges()->begin();
    while ( eit->hasNext() ) {
      eit->next();
      auto s = nodeNodeMap[eit->currentSource()];
      auto t = nodeNodeMap[eit->currentTarget()];
      //auto s = eit->currentSource();
      //auto t = eit->currentTarget();
      //if ( G[s].name == "" ) { G[s].name = GData.getNodeAttribute(s, attributeID); }
      //if ( G[t].name == "" ) { G[t].name = GData.getNodeAttribute(t, attributeID); }
      boost::add_edge(s,t,G);
    }

    Graph::vertex_iterator b,e;
    std::tie(b,e) = boost::vertices(G);
    for ( auto v = b; v < e; ++v ) {
      std::cerr << "node : " << G[*v].name << " -> " << *v << "\n";
    }


    using actors::VertexDescriptor;
    std::mutex m;
    auto q = tbb::concurrent_bounded_queue<VertexDescriptor>();

    /*
    using actors::VertexDescriptor;

    auto gp = shared_ptr<Graph>(&G);
    
    q.set_capacity( lemon::countNodes(G) );

    //actors::SubgraphSpectrumWorker<Graph,tbb::concurrent_bounded_queue<VertexDescriptor>> worker(gp, q);    
    
    using std::vector;
    using std::unordered_map;
    */
   
    tbb::task_scheduler_init init(1); //creates 4 threads

    tbb::parallel_for_each( b, e, [&](const VertexID& n) {
      actors::SubgraphSpectrumWorker<Graph,tbb::concurrent_bounded_queue<actors::VertexDescriptor>> a;
      std::cerr << "computing signature for vertex " << n << "\n";
      a.compute(G, q, n, m);
     }
    );

    std::thread result( 
      [&]() {

        size_t numRemaining = boost::num_vertices(G);
        VertexDescriptor vd;
        while( numRemaining > 0 ){
          q.pop(vd);
          auto s = "hello " + boost::lexical_cast<std::string>( vd.id ) + " world\n";
          std::cerr << "spectrum for vertex : " << vd.id << " =";
          for ( auto& e : vd.spectra.back() ) { std::cerr << " " << e; } std::cerr << "\n";
          --numRemaining;
        }

      }
      );

    return 0;
    
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  return 0;
}
