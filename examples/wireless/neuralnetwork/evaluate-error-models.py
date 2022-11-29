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

if args.dimensionalPacketlevel:
    print("Dimensional Packetlevel")
    os.system(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalPacketlevel")

if args.dimensionalPacketlevelNeuralNetwork:
    print("Dimensional Packetlevel Neural Network")
    os.system(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork")

if args.dimensionalSymbollevel:
    print("Dimensional Symbollevel")
    os.system(f"opp_runall -b 1 -j 8 {INET} -f {args.inifile} -u Cmdenv -c DimensionalSymbollevel")
