#!/bin/bash

# stop docker containers
sudo docker stop c0
sudo docker stop c1
sudo docker stop c2

# disconnect docker private networks from docker containers
sudo docker network disconnect net0 c0
sudo docker network disconnect net1 c1
sudo docker network disconnect net2 c2

# destroy docker private networks
sudo docker network rm net0
sudo docker network rm net1
sudo docker network rm net2

# desctroy docker containers
sudo docker rm c0
sudo docker rm c1
sudo docker rm c2

# destroy TAP interface
sudo ip tuntap del mode tap dev tap0
sudo ip tuntap del mode tap dev tap1
sudo ip tuntap del mode tap dev tap2

