#!/bin/sh

inet_dbg -s -u Cmdenv -c FifoQueueing --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c PriorityQueueing --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c FramePreemption --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c RealisticFifoQueueing --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c RealisticPriorityQueueing --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
inet_dbg -s -u Cmdenv -c RealisticFramePreemption --sim-time-limit=0.1s --cmdenv-redirect-output=true --record-eventlog=true &
