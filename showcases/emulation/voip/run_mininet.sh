#!/bin/sh

# run python script; but save original $PATH and $USER before that for use by mininet-veth.py
export SAVED_PATH=$PATH && export SAVED_USER=$USER && sudo -E python3 mininet-veth.py
