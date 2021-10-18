#!/bin/sh

inet_dbg -s -u Cmdenv -c Basic --sim-time-limit=0.1s --cmdenv-redirect-output=true
inet_dbg -s -u Cmdenv -c AnyLocation --sim-time-limit=0.1s --cmdenv-redirect-output=true
