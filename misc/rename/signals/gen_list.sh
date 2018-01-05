#!/bin/sh

#find ../../../src -name "*.cc" -o -name "*.h" -o -name "*.ned" -o -name "*.msg" | grep -v ../../../src/inet/transportlayer/tcp_lwip/lwip  | grep -v ../../../src/inet/routing/extras/ >filelist.txt
find ../../../src -name "*.cc" -o -name "*.h" | grep -v ../../src/inet/transportlayer/tcp_lwip/lwip  | grep -v ../../../src/inet/routing/extras/ | grep -v /sctp/ >filelist.txt
