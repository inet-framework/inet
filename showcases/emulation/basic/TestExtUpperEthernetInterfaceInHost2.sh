#!/bin/bash

# create TAP interface
sudo ip tuntap add mode tap dev tapa

# assign IP addresses to interface
sudo ip addr add 192.168.2.2/24 dev tapa

# bring up interface
sudo ip link set dev tapa up

# run simulation
inet -u Cmdenv -c ExtUpperEthernetInterfaceInHost2 --sim-time-limit=2s &> inet.out

# check output
if grep -q "from 192.168.2.2" "inet.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

# destroy TAP interface
sudo ip tuntap del mode tap dev tapa

