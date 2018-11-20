#!/bin/bash

# create TAP interface
sudo ip tuntap add mode tap dev tap0

# assign IP addresses to interface
sudo ip addr add 192.168.2.1/24 dev tap0

# bring up interface
sudo ip link set dev tap0 up

# destroy TAP interface
sleep 1
sudo ip tuntap del mode tap dev tap0

