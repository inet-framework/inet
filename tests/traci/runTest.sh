#!/bin/sh

opp_run -l../../src/inet -n"../../src;." -u Cmdenv "$@" | egrep -i "^(Pass|FAIL)"
