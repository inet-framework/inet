#!/bin/sh

# module tests
cd module
./runtest GridNeighborCache.test QuadTreeNeighborCache.test NeighborListCache.test
cd ..

