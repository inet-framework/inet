#!/bin/sh

find ../../src -name "*.cc" -o -name "*.h" | grep -v ../../src/transportlayer/tcp_lwip/lwip  | grep -v ../../src/routing/extras/ >filelist.txt
./uncrustify -c omnetpp.style --no-backup -l CPP -F filelist.txt >x1.out 2>x1.err
grep -v 'Parsing:' -B 1 x1.err >x1.err2
