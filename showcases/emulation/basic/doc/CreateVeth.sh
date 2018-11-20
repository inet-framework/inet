#!/bin/bash

# create virtual ethernet link: veth0 <--> veth1
sudo ip link add veth0 type veth peer name veth1

# veth0 <--> veth1 link uses 192.168.2.x addresses
sudo ip addr add 192.168.2.2/24 dev veth1

# bring up both interfaces
sudo ip link set veth0 up
sudo ip link set veth1 up

# destroy virtual ethernet link
sleep 1
sudo ip link del veth0

