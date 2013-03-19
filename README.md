GHOST-CPP
=========

Rewriting (at least some) of GHOST in C++

Right now this repo contains just the code to compute the subgraph spectra.  It
aims to be compatible with the Scala version of ghost by outputting the
signatures in the same format.

Building
--------

First you need to build and install libgexf

```bash
$ cd mylibgexf
$ mkdir build
$ cd build
$ BOOST_ROOT=(path to boost) cmake ..
$ make -j
$ make install
```

That should install a shared library for libgexf in
the lib subdirectory.

Now you need to build the GHOST subgraph extractor:

```bash
$ cd ../../
$ mkdir build
$ cd build
$ BOOST_ROOT=(path to boost) cmake ..
$ make -j
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



