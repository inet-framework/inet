#!/bin/sh

inet_dbg -s -u Cmdenv -c NormalOperation --sim-time-limit=4s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c LinkFailure --sim-time-limit=4s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c Failover --sim-time-limit=4s --cmdenv-redirect-output=true
