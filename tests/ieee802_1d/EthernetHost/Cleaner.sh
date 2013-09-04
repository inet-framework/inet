#!/bin/bash

while read SIM;do
echo "Cleaning $SIM"
rm -f $SIM/results/*
done;
rm -f results/*
