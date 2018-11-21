#!/bin/bash

# create TAP interface
sudo ip tuntap add mode tap dev tap0

# assign IP addresses to interface
sudo ip addr add 192.168.2.1/24 dev tap0

# bring up interface
sudo ip link set dev tap0 up

# run simulation
inet -u Cmdenv -c ExtUpperEthernetInterfaceInHost1 --sim-time-limit=2s &> inet.out &
sleep 1

# ping into simulation
ping -c 2 -W 2 192.168.2.2 > ping.out

# check output
if grep -q "from 192.168.2.2" "ping.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

# destroy TAP interface
sleep 1
sudo ip tuntap del mode tap dev tap0

