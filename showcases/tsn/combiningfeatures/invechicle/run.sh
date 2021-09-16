#!/bin/sh

inet_dbg -s -u Cmdenv -c StandardEthernet --sim-time-limit=5s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c TsnEthernet --sim-time-limit=5s --cmdenv-redirect-output=true
