#!/bin/bash

# create TAP interfaces
sudo ip tuntap add mode tap dev tap0
sudo ip tuntap add mode tap dev tap1
sudo ip tuntap add mode tap dev tap2

# bring up TAP interfaces
sudo ip link set dev tap0 up
sudo ip link set dev tap1 up
sudo ip link set dev tap2 up

# assign IP addresses to interface
sudo ip addr add 192.168.2.1/32 dev tap0
sudo ip addr add 192.168.3.1/32 dev tap1
sudo ip addr add 192.168.4.1/32 dev tap2

# create docker private networks mapped to TAP interfaces
sudo docker network create -d macvlan --ipv6 --subnet=192.168.2.0/24 --subnet 2001:abcd:abcd::2/64 -o parent=tap0 net0
sudo docker network create -d macvlan --ipv6 --subnet=192.168.3.0/24 --subnet 2001:abcd:abcd::3/64 -o parent=tap1 net1
sudo docker network create -d macvlan --ipv6 --subnet=192.168.4.0/24 --subnet 2001:abcd:abcd::4/64 -o parent=tap2 net2

# create and start docker containers
sudo docker run -itd --name c0 --privileged babeld
sudo docker run -itd --name c1 --privileged babeld
sudo docker run -itd --name c2 --privileged babeld

# connect docker containers to private networks
sudo docker network connect net0 c0
sudo docker network connect net1 c1
sudo docker network connect net2 c2

# query docker interface MAC addresses
ADDRESS0=`sudo docker exec c0 ifconfig eth1 | awk '/ether/ {print $2}'`
ADDRESS1=`sudo docker exec c1 ifconfig eth1 | awk '/ether/ {print $2}'`
ADDRESS2=`sudo docker exec c2 ifconfig eth1 | awk '/ether/ {print $2}'`

# set MAC addresses on TAP interfaces
sudo ip link set tap0 address $ADDRESS0
sudo ip link set tap1 address $ADDRESS1
sudo ip link set tap2 address $ADDRESS2



















#sudo ip addr add 169.254.2.0/24 brd 169.254.2.255 scope link dev tap0
#sudo ip addr add 169.254.3.0/24 brd 169.254.3.255 scope link dev tap1
#sudo ip addr add 169.254.4.0/24 brd 169.254.4.255 scope link dev tap2

#sudo docker exec c0 ip route add 192.168.3.0/24 dev eth1
#sudo docker exec c1 ip route add 192.168.2.0/24 dev eth1

#sudo /home/levy/workspace/pipework/pipework --direct-phys tap0 c0 192.168.2.2/24
#sudo /home/levy/workspace/pipework/pipework --direct-phys tap1 c1 192.168.3.2/24

#sudo docker network create --opt com.docker.network.bridge.enable_icc=false --subnet=192.168.2.0/24 net0
#sudo docker network create --opt com.docker.network.bridge.enable_icc=false --subnet=192.168.3.0/24 net1

#sudo brctl addif br-a7b842959d82a847f4c90e8658ba5fe22e9b9803a871eaa0fb37a463abfea0ac tap0
#sudo brctl addif br-6903f121b16d80bf1a0c3a459341ce425e8b0e9637081389e7d9b4d376f8b7f0 tap1

#sudo docker exec c0 ip link set eth1 address 0a:aa:00:00:00:22
#sudo docker exec c1 ip link set eth1 address 0a:aa:00:00:00:33

#sudo docker exec c0 ping 192.168.3.2

#sudo docker attach c0
#sudo docker attach c1

#sudo docker start c0
#sudo docker start c1

#babeld -I tapb.pid -d1 eth1

#sudo ip link set tap0 address 0a:aa:00:00:00:22
#sudo ip link set tap1 address 0a:aa:00:00:00:33

#sudo ip addr add 169.254.2.0/24 brd 169.254.2.255 scope link dev tap0

# disable IPv6 for TAP interfaces
#sudo sysctl -w net.ipv6.conf.tap0.autoconf=1
#sudo sysctl -w net.ipv6.conf.tap0.disable_ipv6=1
#sudo sysctl -w net.ipv6.conf.tap1.autoconf=1
#sudo sysctl -w net.ipv6.conf.tap1.disable_ipv6=1
#sudo sysctl -w net.ipv6.conf.tap2.autoconf=1
#sudo sysctl -w net.ipv6.conf.tap2.disable_ipv6=1


