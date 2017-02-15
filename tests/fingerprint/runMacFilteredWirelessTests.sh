#!/bin/sh
./fingerprints mac-filtered-examples.csv -m wireless -m adhoc -m mobileipv6 -m WirelessDHCP -m examples/aodv -m neighborcache -a fingerprint-events=NOT\\\(**.mac.**\\\) cmdenv-log-prefix=%f\\ %g\\ [%l] record-eventlog=false cmdenv-express-mode=true $*
