#ifndef SUBGRAPH_EXTRACTION_ACTORS_HPP
#define SUBGRAPH_EXTRACTION_ACTORS_HPP

#include <memory>
#include <typeinfo>
#include <locale>
#include <string>

#include <thread>
#include <mutex>
#include "boost/graph/subgraph.hpp"
#include "math/LaplacianCalculator.hpp"

namespace actors {

  using std::shared_ptr;
  using std::vector;

  struct LevelInfo {
    std::vector<std::string> vnames;
    std::vector<double> spectrum;
    double density;
  };

  struct VertexDescriptor {
    std::string id;
    std::unordered_map<size_t, LevelInfo> levelInfo;
    size_t max_level;
  };

  // Our actor lib requires an implementation of ==
  bool operator==( const VertexDescriptor& lhs, const VertexDescriptor& rhs ) {
    return lhs.id == rhs.id;
  }

  template <typename T>
  void swap_endian(T& pX)
  {
    // should static assert that T is a POD

    char& raw = reinterpret_cast<char&>(pX);
    std::reverse(&raw, &raw + sizeof(T));
  }

  /**
   * The following function writes the vertex descriptor to the given stream
   * in a way that is compatible with the Scala version of GHOST.  I'm not
   * certain it's 100% correct yet, and it certainly isn't robust.  Mainly
   * it just swaps the byte order (to big endian which the JVM expects) and
   * writes the strings out in a way compatible with DataOutputStream.writeUTF.
   * This code assumes that all strings it's writing are simple ASCII --- otherwise
   * things will break.
   */
  template<typename Stream>
  bool writeVertexDescriptor( Stream& out, VertexDescriptor& vd ) {
    size_t lastValidLevel = 1;
    
    int16_t slen = vd.id.length();
    swap_endian(slen);
    out.write( reinterpret_cast<const char*>(&slen), sizeof(slen));
    out.write( reinterpret_cast<const char*>(&vd.id[0]), vd.id.length() );

    for ( auto l : boost::irange(size_t(1), vd.max_level+1) ) {
      int32_t tmplvl = l;
      swap_endian(tmplvl);
      out.write( reinterpret_cast<const char*>(&tmplvl), sizeof(tmplvl) );

      int32_t level = (vd.levelInfo.find(l) == vd.levelInfo.end()) ? lastValidLevel : l;
      lastValidLevel = level;

      int32_t numNewVerts = vd.levelInfo[level].vnames.size();
      swap_endian(numNewVerts);
      out.write( reinterpret_cast<const char*>(&numNewVerts), sizeof(numNewVerts) );
      for ( auto vn : vd.levelInfo[level].vnames ) {
        int16_t slen = vn.length();
        swap_endian(slen);
        out.write( reinterpret_cast<const char*>(&slen), sizeof(slen));
        out.write( reinterpret_cast<const char*>(&vn[0]), vn.length() );
      }

      int32_t specLength = vd.levelInfo[level].spectrum.size();
      swap_endian(specLength);
      out.write( reinterpret_cast<const char*>(&specLength), sizeof(specLength) );
      for( auto s : vd.levelInfo[level].spectrum ) {
        int64_t stmp;
        memcpy(&stmp, &s, sizeof(stmp));
        swap_endian(stmp);
        out.write( reinterpret_cast<const char*>(&stmp), sizeof(stmp) );
      }

      int64_t density;
      memcpy(&density, &vd.levelInfo[level].density, sizeof(density));
      swap_endian(density);
      out.write( reinterpret_cast<const char*>(&density), sizeof(density) );
    }
  }


  template <typename Graph, typename OutQueue>
  class SubgraphSpectrumWorker {
    typedef typename Graph::vertex_descriptor VertexID;
    typedef typename Graph::edge_descriptor   EdgeID;
  public:
    SubgraphSpectrumWorker() {}

    void compute( Graph& G, size_t maxHop, OutQueue& _q, const VertexID& n,  std::mutex& graphM ) const {
      VertexDescriptor vd; vd.id = G[n].name;
      vd.max_level = maxHop;

      auto source = n;

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
      nodes.insert(source);

      // For each level
      size_t order = 0; size_t size = 0;
      size_t maxLevel = maxHop;
      for ( size_t i = 0; i < maxLevel; ++i ) {

       size_t pnum = 0;
       for ( auto& subNode : current ) {
         typename Graph::adjacency_iterator bn, en;
         auto globalSrc = subG.local_to_global(subNode);
         std::tie(bn,en) = boost::adjacent_vertices(globalSrc, G);
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

       auto begEnd = vertices(subG);

       if ( peripheryNodes.size() > 0 ) {
         LaplacianCalculator<Graph> lc( subG );
         auto v = lc.getSpectrum();

         auto density = static_cast<double>( boost::num_vertices(subG) ) / boost::num_edges(subG);

         std::vector<std::string> peripheryNodeNames( peripheryNodes.size() );
         for ( auto i : boost::irange(size_t(0), peripheryNodes.size()) ){
          peripheryNodeNames[i] = subG[peripheryNodes[i]].name;
         }
        vd.levelInfo[i+1] = { peripheryNodeNames, v, density };

        std::swap(current, peripheryNodes);
        peripheryNodes.clear();
       }

     } // end iteration over all levels

     while( !_q.try_push( vd ) ){}
     graphM.lock();
     G.m_children.erase(subGIt);
     graphM.unlock();

    } // end init

  };

}

#endif //SUBGRAPH_EXTRACTION_ACTORS_HPP
