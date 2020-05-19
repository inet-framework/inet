#!/bin/bash

if [[ $# < 1 ]];
then
echo "arguments: device"
else
DEV=$1
sudo ip tuntap del mode tap dev $DEV
fi
