#!/bin/sh

# run python script; but save original $PATH, $USER and $GROUP before that for use by mininet-tap.py
export SAVED_PATH=$PATH && export SAVED_USER=$USER && export SAVED_GROUP=$(id -ng) && sudo -E python3 mininet-tap.py