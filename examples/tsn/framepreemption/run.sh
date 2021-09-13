#!/bin/sh

inet_dbg -s -u Cmdenv -c NormalOperation --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c FramePreemption --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true
