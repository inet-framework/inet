#!/bin/bash

# create TUN interface
sudo ip tuntap add mode tun dev tuna

# assign IP addresses to interface
sudo ip addr add 192.168.2.2/24 dev tuna

# bring up interface
sudo ip link set dev tuna up

# run simulation
inet -u Cmdenv -c ExtUpperIpv4NetworkLayerInHost2 &> inet.out &

# check output
if grep -q "from 192.168.2.2" "inet.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
#rm *.out

# destroy TUN interface
sudo ip tuntap del mode tun dev tuna
