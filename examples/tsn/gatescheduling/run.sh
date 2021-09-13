#!/bin/sh

inet_dbg -s -u Cmdenv -c AlwaysOpen --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c Simple --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c SAT --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c TSNsched --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true
