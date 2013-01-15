#!/bin/bash

if [ $# -eq 0 ]; then
ls | egrep '^Test[0-9]+$' | ./CompareLauncher.sh
elif [ $1 = "clean" ]; then
echo "Cleaning results"
rm -f results/*
ls --hide="[^Test]*" | ./Cleaner.sh
fi
