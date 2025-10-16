#!/bin/bash

# run simulation
inet -u Cmdenv -c ExtLowerUdpInReceiver --sim-time-limit=2s &> inet.out &
sleep 1

# run python script
python UdpSend.py

# check output
if grep -q "" ""; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

