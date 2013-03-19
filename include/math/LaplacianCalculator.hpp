#ifndef LAPLACIAN_CALCULATOR_HPP
#define LAPLACIAN_CALCULATOR_HPP

#include <vector>
#include <cmath>
#include <boost/graph/adjacency_list.hpp>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include "redsvd/redsvd.hpp"

template <typename Graph>
class LaplacianCalculator {
public:
  LaplacianCalculator( Graph& G ) : _G(G) {}

  std::vector<double> getSpectrum() {
    typedef Eigen::SparseMatrix<float> SparseMatT;
    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
    typedef Eigen::Triplet<float> TripletT;

    auto n = boost::num_vertices(_G);
    auto m = boost::num_edges(_G);
    if (n == 1) { return std::vector<double>(1, 0.0); }
    //std::cerr << "M = " << m << "\n";
    SparseMatT L(n,n);
    std::vector< TripletT > triplets;
    triplets.reserve( n+m );

    // indices[id] = i <- the order in which it was visited
    auto N = boost::num_vertices(_G);
    std::vector<int> indices(N, 0);
    std::vector<int> degrees(N, 0);
    size_t k = 0;

    typedef typename Graph::vertex_iterator VertexIt;
    typedef typename Graph::adjacency_iterator AdjacencyIterator;
    VertexIt b,e;
    std::tie(b,e) = boost::vertices(_G);

    for( auto v = b; v != e; ++v ) {
      degrees[*v] = boost::out_degree(*v, _G);
      if ( degrees[*v] == 0 ) {
        std::cerr << "deg(" << *v << ") = 0!\n";
      }
    }
    //std::cerr << " n = " << n << ", k = " << k << "\n";
    //assert( n == k );

    for( auto u = b; u != e; ++u ) {
      //auto centerNodeInd = _G.id(i);
      auto rowInd = *u;
      assert( degrees[rowInd] > 0 );
      AdjacencyIterator nb,ne;
      std::tie(nb,ne) = boost::adjacent_vertices(*u, _G);
      for( auto v = nb; v != ne; ++v ) {
        if ( *u != *v )  {
          //auto otherNodeInd = *otherVert;
          auto colInd = *v;
          triplets.push_back( TripletT(rowInd, colInd, -(1.0 / sqrtf( degrees[rowInd] * degrees[colInd]))) );
        }
      }
      triplets.push_back( TripletT(rowInd, rowInd, 1.0f) );

    }
    // Fill in the matrix
    L.setFromTriplets( triplets.begin(), triplets.end() );

    //REDSVD::RedSymEigen es(L, L.rows()-1);
    MatrixXd DL(L);
    //Eigen::SelfAdjointEigenSolver< MatrixXd > es( DL );
    //es.computeDirect(L, Eigen::DecompositionOptions::EigenvaluesOnly );
    auto evals = DL.selfadjointView<Eigen::Upper>().eigenvalues();

    auto numEvals = evals.rows();
    std::vector<double> ev(numEvals, 0.0);
    for (size_t i = 0; i < numEvals; ++i) { ev[i] = evals(i); }

    return ev;
  }

private:
  Graph& _G;

};

#endif // LAPLACIAN_CALCULATOR_HPP
