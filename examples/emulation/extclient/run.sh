#!/bin/bash

# create virtual ethernet link: veth0 <--> veth1
sudo ip link add veth0 type veth peer name veth1

# veth0 <--> veth1 link uses 192.168.2.x addresses
sudo ip addr add 192.168.2.1 dev veth0
sudo ip addr add 192.168.2.2 dev veth1

# bring up both interfaces
sudo ip link set veth0 up
sudo ip link set veth1 up

# run simulation
inet_dbg -u Cmdenv -c Server &
inet_dbg -u Cmdenv -c Client

# destroy virtual ethernet link
sudo ip link del veth0
