#!/bin/sh

inet_dbg -s -u Cmdenv -c OneMasterClock --sim-time-limit=1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c PrimaryAndHotStandbyMasterClocks --sim-time-limit=1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c TwoMasterClocksExploitingNetworkRedundancy --sim-time-limit=1s --cmdenv-redirect-output=true
