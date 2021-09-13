#!/bin/sh

inet_dbg -s -u Cmdenv -c ManualRedundancy --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c StreamRedundancyConfigurator --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c TsnConfigurator --sim-time-limit=5s --cmdenv-redirect-output=true --record-eventlog=true
