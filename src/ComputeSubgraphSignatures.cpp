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
#include <boost/iostreams/device/file_descriptor.hpp> 
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <libgexf/libgexf.h>

#include <lemon/smart_graph.h>
#include <lemon/concepts/graph.h>

#include "g2logworker.h"
#include "g2log.h"

#include "tclap/CmdLine.h"

#include "tbb/concurrent_queue.h"
#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include "actors/SubgraphExtractionActors.hpp"
#include "ezETAProgressBar.hpp"
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
    TCLAP::ValueArg<string> graphName("i", "input", "graph filename", true, "", "string");
    TCLAP::ValueArg<string> outputName("o", "out", "output filename", true, "", "string");
    TCLAP::ValueArg<size_t> maxHop("k", "khop", "maximum neighborhood distance", true, 3, "int");
    TCLAP::ValueArg<size_t> numProc("p", "nproc", "number of threads to use", true, 10, "int");
    cmd.add(graphName);
    cmd.add(maxHop);
    cmd.add(numProc);
    cmd.add(outputName);

    cmd.parse(argc, argv);

    string logFilePath(".");
    g2LogWorker logger(argv[0], logFilePath);
    g2::initializeLogging(&logger);

    auto GContainer = read(graphName.getValue());
    auto IG = GContainer.getUndirectedGraph();
    auto GData = GContainer.getData();

    auto maxHopDistance = maxHop.getValue();
    auto numThreads = numProc.getValue();

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

    Graph G(IG.getNodeCount());

    auto attributeID = findAttributeID( GData, "gname" );

    
    std::map<libgexf::t_id, VertexID> nodeNodeMap;

    const auto& nit = IG.getNodes()->begin();
    size_t nodeCnt = 0;
    while ( nit->hasNext() ) {
      auto nid = nit->next();
      auto name = GData.getNodeAttribute(nid, attributeID);
      LOG(INFO) << "node : " << name << "\n";
      nodeNodeMap[nid] = nodeCnt;
      G[nodeCnt].name = name;
      ++nodeCnt;
    }

    const auto& eit = IG.getEdges()->begin();
    while ( eit->hasNext() ) {
      eit->next();
      auto s = nodeNodeMap[eit->currentSource()];
      auto t = nodeNodeMap[eit->currentTarget()];
      boost::add_edge(s,t,G);
    }

    Graph::vertex_iterator b,e;
    std::tie(b,e) = boost::vertices(G);
    for ( auto v = b; v < e; ++v ) {
      LOG(INFO) << "node : " << G[*v].name << " -> " << *v << "\n";
    }


    using actors::VertexDescriptor;
    std::mutex m;
    auto q = tbb::concurrent_bounded_queue<VertexDescriptor>();

    ez::ezETAProgressBar eta(boost::num_vertices(G));

    eta.start();

    std::thread result( 
      [&]() {

        boost::iostreams::filtering_ostream out; 
        out.push(boost::iostreams::gzip_compressor()); 
        out.push(boost::iostreams::file_descriptor_sink(outputName.getValue())); 


        //std::ofstream outfile( outputName.getValue() );
        int32_t numVerts = boost::num_vertices(G);
        actors::swap_endian(numVerts);
        out.write( reinterpret_cast<const char*>(&numVerts), sizeof(numVerts) );
        int32_t numHops = maxHopDistance;
        actors::swap_endian(numHops);
        out.write( reinterpret_cast<const char*>(&numHops), sizeof(numHops) );

        //outfile << boost::num_vertices(G) << '\n';
        //outfile << maxHopDistance << '\n';

        size_t numRemaining = boost::num_vertices(G);
        VertexDescriptor vd;
        while( numRemaining > 0 ){
          while( ! q.try_pop(vd) ) {}
            writeVertexDescriptor( out, vd );
            /*
            outfile << vd.id << '\n';
          size_t lastValidLevel = 0;
          for( auto level : boost::irange(size_t(0), maxHopDistance) ) {
            if ( vd.levelInfo.find(level) != vd.levelInfo.end() ) {
              outfile << vd.levelInfo[level].spectrum.size() << '\n';
              for( auto v : vd.levelInfo[level].spectrum ) { outfile << v << ' '; }
              outfile << '\n';
              lastValidLevel = level;
            } else {
              outfile << vd.levelInfo[lastValidLevel].spectrum.size() << '\n';
              for( auto v : vd.levelInfo[lastValidLevel].spectrum ) { outfile << v << ' '; }
              outfile << '\n';              
            }
            
          }*/
          --numRemaining;
          ++eta;
        }

        //outfile.close();
        //out.close();
      }
     );

   
    tbb::task_scheduler_init init(numThreads); //creates numThreads different threads

    tbb::parallel_for_each( b, e, [&](const VertexID& n) {
      actors::SubgraphSpectrumWorker<Graph,tbb::concurrent_bounded_queue<actors::VertexDescriptor>> a;
      a.compute(G, maxHopDistance, q, n, m);
     }
    );


    result.join();
    return 0;
    
  } catch (TCLAP::ArgException &e) {
    LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  return 0;
}
