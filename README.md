GHOST-CPP
=========

Rewriting (at least some) of GHOST in C++

Right now this repo contains just the code to compute the subgraph spectra.  It
aims to be compatible with the Scala version of ghost by outputting the
signatures in the same format.

Building
--------

The build process relies on a recent version of the [cmake](www.cmake.org)
build tool.  Compilation has only been tested under [g++](gcc.gnu.org), of which
you should be using a recent version that supports c++11.  Finally, you need to 
install [Boost](www.boost.org) (version >= 1.53) --- if you're on a Mac, it is 
essential that you build boost with the same compiler you'll use to build GHOST.
This means that you should compile Boost with g++, rather than the version of 
Clang that comes with OSX.  Once these dependencies are satisfied, you can
build GHOST by running the following commands from the top-level directory.

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_CXX_COMPILER=g++-mp-4.8 -DBOOST_ROOT=<PATH_TO_BOOST> ..
$ make 
$ cp src/ComputeSubgraphSignatures .
```

now the build directory should include a `ComputeSubgraphSignatures` executable.
You should be able to run this as follows:

```bash
$ ./ComputeSubgraphSignatures -i <inputgraph> -k <num hops> -o <sigfile> -p <# procs to use>
```

This will compute the signatures for `<inputgraph>` and write the output to `<sigfile>`.
From that point forward, everything should be the same in terms of using the 
original Scala version of GHOST.

