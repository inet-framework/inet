#!/bin/sh

inet_dbg -s -u Cmdenv -c NoClockDrift --sim-time-limit=0.1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c ConstantClockDriftRate --sim-time-limit=0.1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c OutOfBandSynchronization --sim-time-limit=0.1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c GptpSynchronization --sim-time-limit=0.1s --cmdenv-redirect-output=true
