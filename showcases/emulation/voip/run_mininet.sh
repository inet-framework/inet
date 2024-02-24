#!/bin/sh
export SAVED_PATH=$PATH && export SAVED_USER=$USER && sudo -E python3 mininet-veth.py
