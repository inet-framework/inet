#!/bin/bash

# run simulation
inet -u Cmdenv -c ExtLowerUdpInSender --sim-time-limit=2s &> inet.out &
sleep 1

# run python script
python UdpReceive.py

# check output
if grep -q "" ""; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

