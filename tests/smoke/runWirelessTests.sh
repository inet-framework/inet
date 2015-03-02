#!/bin/sh
./smoketest -c -m wireless
./smoketest -c -m adhoc
./smoketest -c -m manet 
./smoketest -c -m mobileipv6
./smoketest -c -m WirelessDHCP
