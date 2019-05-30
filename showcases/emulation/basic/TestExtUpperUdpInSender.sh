#!/bin/bash

# TODO: ExtUpperUdp is not yet implemented

# run simulation
inet -u Cmdenv -c ExtUpperUdpInSender &> inet.out &

# check output
if grep -q "" ""; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

