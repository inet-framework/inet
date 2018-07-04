#!/bin/sh

DEV=tap10
sudo ip tuntap del mode tap dev $DEV
