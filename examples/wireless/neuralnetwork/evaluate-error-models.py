#!/usr/bin/python3

import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--debug", default=False, action='store_true')
parser.add_argument("-p", "--dimensionalPacketlevel", default=False, action='store_true')
parser.add_argument("-n", "--dimensionalPacketlevelNeuralNetwork", default=False, action='store_true')
parser.add_argument("-s", "--dimensionalSymbollevel", default=False, action='store_true')
parser.add_argument("inifile")
args = parser.parse_args()

INET = "inet_dbg" if args.debug else "inet"

ADD_ARGS = "--cmdenv-express-mode=false --cmdenv-log-level=trace --cmdenv-redirect-output=false --cmdenv-log-level=TRACE"

import subprocess

procs = []

if args.dimensionalPacketlevel:
    print("Dimensional Packetlevel")
    p = subprocess.Popen(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalPacketlevel " + ADD_ARGS, shell=True)
    procs.append(p)

if args.dimensionalPacketlevelNeuralNetwork:
    print("Dimensional Packetlevel Neural Network")
    p = subprocess.Popen(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork " + ADD_ARGS, shell=True)
    procs.append(p)

if args.dimensionalSymbollevel:
    print("Dimensional Symbollevel")
    p = subprocess.Popen(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalSymbollevel " + ADD_ARGS, shell=True)
    procs.append(p)

for p in procs:
    p.wait()
