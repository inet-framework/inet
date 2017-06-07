#!/bin/sh
./smoketest -c -m wireless -m Wireless -m adhoc -m mobileipv6 -m dymo -m examples/aodv -m neighborcache -m objectcache $*
