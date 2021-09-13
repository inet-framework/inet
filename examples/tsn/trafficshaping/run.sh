#!/bin/sh

inet_dbg -s -u Cmdenv -c CreditBasedShaper --sim-time-limit=0.1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c TimeAwareShaper --sim-time-limit=0.1s --cmdenv-redirect-output=true &
inet_dbg -s -u Cmdenv -c AsynchronousShaper --sim-time-limit=0.1s --cmdenv-redirect-output=true
