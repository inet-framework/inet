#!/bin/bash

# run simulation
inet -u Cmdenv -c ExtLowerIpv4NetworkLayerInHost2 &> inet.out &

# check output
if grep -q "" ""; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

