#!/bin/sh

# module tests
cd module
./runtest GridCacheTest.test QuadTreeCache.test NeighborListCache.test
cd ..

