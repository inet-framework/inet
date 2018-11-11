#!/bin/bash

# create TUN interface
sudo ip tuntap add mode tun dev tun0

# assign IP addresses to interface
sudo ip addr add 192.168.2.2/24 dev tun0

# bring up interface
sudo ip link set dev tun0 up

# run simulation
inet -u Cmdenv -c ExtUpperIpv4NetworkLayerInHost2 &> inet.out &

# check output
if grep -q "from 192.168.2.2" "inet.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

# destroy TUN interface
sleep 1
sudo ip tuntap del mode tun dev tun0
