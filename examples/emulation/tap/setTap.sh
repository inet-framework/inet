#!/bin/sh

DEV=tap10
sudo ip tuntap add mode tap dev $DEV
sudo ip addr add 10.0.0.2/24 dev $DEV  # give it an ip
sudo ip link set dev $DEV up  # bring the if up
ip route get 10.0.0.2  # check that packets to 10.0.0.x are going through tun0
echo ping 10.0.0.1  # leave this running in another shell to be able to see the effect of the next example
