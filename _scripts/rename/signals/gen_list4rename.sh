#!/bin/sh

#find ../../src -name "*.cc" -o -name "*.h" -o -name "*.ned" -o -name "*.msg" | grep -v ../../src/inet/transportlayer/tcp_lwip/lwip  | grep -v ../../src/inet/routing/extras/ >filelist.txt
find ../../../src -name "*.cc" -o -name "*.h" -o -name "*.msg" -o -name "*.ned" | grep -v ../../../src/inet/transportlayer/tcp_lwip/lwip  | grep -v ../../src/inet/routing/extras/ | grep -v /sctp/ | grep -v '_m.' | sort >filelist4rename.txt
find ../../../tests -name "*.cc" -o -name "*.h" -o -name "*.msg" -o -name "*.ned" -o -name "*.test" | grep -v '_m.' | grep -v '/work/' | sort >>filelist4rename.txt
