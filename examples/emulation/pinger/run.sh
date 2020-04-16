#!/bin/bash

# create virtual ethernet link: veth0 <--> veth1
sudo ip link add veth0 type veth peer name veth1

# veth0 <--> veth1 link uses 192.168.2.x addresses
sudo ip addr add 192.168.2.2 dev veth1

# bring up both interfaces
sudo ip link set veth0 up
sudo ip link set veth1 up

# add routes for new link
sudo route add -net 192.168.2.0 netmask 255.255.255.0 dev veth1

# run simulation
inet_dbg -u Cmdenv

# destroy virtual ethernet link
sudo ip link del veth0
