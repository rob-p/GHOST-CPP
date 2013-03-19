#ifndef SUBGRAPH_EXTRACTION_ACTORS_HPP
#define SUBGRAPH_EXTRACTION_ACTORS_HPP

#include <memory>
#include <typeinfo>

#include "lemon/adaptors.h"
#include "boost/lexical_cast.hpp"
#include <thread>
#include <mutex>
#include "boost/graph/subgraph.hpp"
  //#include "cppa/cppa.hpp"

#include "math/LaplacianCalculator.hpp"

namespace actors {

  using std::shared_ptr;
  using std::vector;
  // using cppa::on;
  // using cppa::others;
  // using cppa::actor_ptr;
  // using cppa::atom;
  // using cppa::event_based_actor;

  struct VertexDescriptor {
    int id;
    vector<vector<double>> spectra;
  };

  // Our actor lib requires an implementation of ==
  bool operator==( const VertexDescriptor& lhs, const VertexDescriptor& rhs ) {
    return lhs.id == rhs.id;
  }

  // class VertexDescriptorReceiverActor : public event_based_actor {
  //   //typedef typename Graph::NodeIt NodeIt;
  //   //typedef typename Graph::Node Node;
  // public:
  //   VertexDescriptorReceiverActor(){}

  //   void init() {
  //     become(
  //            on<atom("descriptor"), VertexDescriptor>() >> [=] (const VertexDescriptor& vd) {
  //              auto s = "hello " + boost::lexical_cast<std::string>( vd.id ) + " world\n";
  //              //reply( atom("response"), s);
  //              std::cerr << "spectrum for vertex : " << vd.id << " =";
  //              for ( auto& e : vd.spectra.back() ) { std::cerr << " " << e; } std::cerr << "\n";

  //            }
  //            ); // end become
  //   } // end init

  // };




  template <typename Graph, typename OutQueue>
  class SubgraphSpectrumWorker {
    //typedef typename Graph::NodeIt NodeIt;
    typedef typename Graph::vertex_descriptor VertexID;
    typedef typename Graph::edge_descriptor   EdgeID;
  public:
    //SubgraphSpectrumWorker( shared_ptr<Graph>& G, OutQueue& q ) : _G(G), _q(q) { }
    SubgraphSpectrumWorker() {}

    void compute( Graph& G, OutQueue& _q, const VertexID& n,  std::mutex& graphM ) const {
     //std::cerr << "Computing spectra for vertex " << n << "\n";
     
     VertexDescriptor vd; vd.id = n;

     auto source = n;//(this->_G)->nodeFromId(n);

     // Our current subgraph
     typedef typename boost::subgraph<Graph> SubGraph;

     // lock the graph to create the subgraph
     graphM.lock();
     Graph subG = G.create_subgraph();
     auto subGIt = G.m_children.end();
     subGIt--;
     // freedom!
     graphM.unlock();

     // Doing our BFS from the source we need these
     std::vector< VertexID > peripheryNodes;
     std::vector< EdgeID > peripheryEdges;
     std::vector< VertexID > current(1, boost::add_vertex(source, subG));
     std::set< VertexID > nodes;
     nodes.insert(current[0]);

     // For each level
     size_t order = 0; size_t size = 0;
     size_t maxLevel = 4;
     for ( size_t i = 0; i < maxLevel; ++i ) {
      //std::cerr << "v = " << n << ", level = " << i << "\n";
       for ( auto& subNode : current ) {
         typename Graph::adjacency_iterator bn, en;
         auto globalSrc = subG.local_to_global(subNode);
         std::tie(bn,en) = boost::adjacent_vertices( globalSrc, G);
         for( auto globalTgt = bn;  globalTgt != en; ++globalTgt ) {
           if ( (*globalTgt) != globalSrc ) {
            if ( nodes.find(*globalTgt) == nodes.end() ) {
             auto tgt = boost::add_vertex(*globalTgt, subG);
             peripheryNodes.push_back(tgt);
             nodes.insert(*globalTgt);
           } 
           }
         }
       }

       //for ( auto& node : peripheryNodes ) { subG.enable(node); if ( !subG.status(node) ) { order++; } }
       //for ( auto& edge : peripheryEdges ) { subG.enable(edge); if ( !subG.status(edge) ) { size++; } }

       std::swap(current, peripheryNodes);
       peripheryNodes.clear();

       std::cerr << "computing decomposition of subgraph with order = " <<
                     boost::num_vertices(subG) << ", size = " << boost::num_edges(subG) << "\n";
       LaplacianCalculator<Graph> lc( subG );
       auto v = lc.getSpectrum();
       vd.spectra.push_back(v);

       std::cerr << "computed sigma_{" << i << "}("<< n <<")\n";
     } // end iteration over all levels

     while( _q.try_push( vd ) ){}
     graphM.lock();
     G.m_children.erase(subGIt);
     graphM.unlock();

    } // end init

  // private:
  //   shared_ptr<Graph> _G;
  //   OutQueue& _q;
  };


  // template <typename Graph>
  // class SubgraphExtractionActor : public event_based_actor {
  //   //typedef typename Graph::NodeIt NodeIt;
  //   //typedef typename Graph::Node Node;
  // public:
  //   SubgraphExtractionActor( shared_ptr<Graph>& G, actor_ptr dr ) : _G(G), _dr(dr) { }

  //   void init() {
  //     become(
  //            on<atom("compute"), int>() >> [=] (const int& n) {
  //              VertexDescriptor vd;
  //              vd.id = n;

  //              auto source = (this->_G)->nodeFromId(n);

  //              // Our current subgraph
  //              typedef typename Graph::template NodeMap<bool> BoolNodeMap;
  //              BoolNodeMap subVerts( *((this->_G).get()) );
  //              typedef typename Graph::template EdgeMap<bool> BoolEdgeMap;
  //              BoolEdgeMap subEdges( *((this->_G).get()) );
  //              lemon::FilterNodes<Graph> subG( *((this->_G).get()), subVerts);

  //              // Doing our BFS from the source we need these
  //              std::vector< typename Graph::Node > peripheryNodes;
  //              std::vector< typename Graph::Edge > peripheryEdges;
  //              std::vector< typename Graph::Node > current(1, source);
  //              subVerts[source] = true;

  //              // For each level
  //              size_t order =0; size_t size = 0;
  //              size_t maxLevel = 4;
  //              for ( size_t i = 0; i < maxLevel; ++i ) {
  //                for ( auto& node : current ) {
  //                  for( typename Graph::OutArcIt e(*((this->_G).get()), node); e != lemon::INVALID; ++e ) {
  //                    auto otherNode = (this->_G)->target(e);
  //                    peripheryNodes.push_back(otherNode);
  //                    peripheryEdges.push_back(e);
  //                  }
  //                }

  //                for ( auto& node : peripheryNodes ) { subG.enable(node); if ( !subG.status(node) ) { order++; } }
  //                //for ( auto& edge : peripheryEdges ) { subG.enable(edge); if ( !subG.status(edge) ) { size++; } }

  //                std::swap(current, peripheryNodes);
  //                peripheryEdges.clear();

  //                LaplacianCalculator< lemon::FilterNodes<Graph> > lc( subG );
  //                auto v = lc.getSpectrum();
  //                vd.spectra.push_back(v);
  //              }

  //              send( this->_dr, atom("descriptor"), vd );
  //              //std::cerr << "computing subgraph for node " << (this->_G)->id(n) << "\n";
  //            },
  //            others() >> [=]() {
  //              reply( atom("response"), "wtf\n" );
  //            }

  //            ); // end become
  //   } // end init

  // private:
  //   shared_ptr<Graph> _G;
  //   actor_ptr _dr;
  // };

}

#endif //SUBGRAPH_EXTRACTION_ACTORS_HPP
