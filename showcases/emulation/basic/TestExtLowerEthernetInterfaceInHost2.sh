#!/bin/bash

# create virtual ethernet link: vetha <--> vethb
sudo ip link add vetha type veth peer name vethb

# vetha <--> vethb link uses 192.168.2.x addresses
sudo ip addr add 192.168.2.1 dev vetha
sudo ip addr add 192.168.2.2 dev vethb

# bring up both interfaces
sudo ip link set vetha up
sudo ip link set vethb up

# add routes for new link
sudo route add -net 192.168.2.0 netmask 255.255.255.0 dev vethb

# run simulation
inet -u Cmdenv -c ExtLowerEthernetInterfaceInHost2 --sim-time-limit=2s &> inet.out &

# ping into simulation
ping -c 2 -W 2 192.168.2.2 > ping.out

# check output
if grep -q "from 192.168.2.2" "ping.out"; then echo $0 ": PASS"; else echo $0 ": FAIL"; fi
rm *.out

# destroy virtual ethernet link
sudo ip link del vetha

