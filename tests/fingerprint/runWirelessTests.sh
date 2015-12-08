#!/bin/sh
./fingerprints *.csv -m wireless -m adhoc -m mobileipv6 -m WirelessDHCP -m examples/aodv -m neighborcache $*
