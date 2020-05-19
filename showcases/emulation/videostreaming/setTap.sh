#!/bin/bash

if [[ $# < 2 ]];
then
echo "arguments: device address"
else
DEV=$1
ADDR=$2
sudo ip tuntap add mode tap dev $DEV
sudo ip addr add $ADDR/24 dev $DEV  # give it an ip
sudo ip link set dev $DEV up  # bring the if up

echo 2 | sudo tee /proc/sys/net/ipv4/conf/$DEV/rp_filter
echo 1 | sudo tee /proc/sys/net/ipv4/conf/$DEV/accept_local

ip route get $ADDR  # check that packets to 10.0.0.x are going through tun0
fi
