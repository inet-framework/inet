#!/bin/sh

inet $@ -s -u Cmdenv -c StandardEthernet --sim-time-limit=1s --cmdenv-redirect-output=true &
#inet $@ -s -u Cmdenv -c ManualTsn --sim-time-limit=1s --cmdenv-redirect-output=true
#inet $@ -s -u Cmdenv -c AutomaticTsn --sim-time-limit=1s --cmdenv-redirect-output=true
