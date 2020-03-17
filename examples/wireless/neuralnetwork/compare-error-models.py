#!/usr/bin/python3

import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-s", "--scalarPacketlevel", default=False)
parser.add_argument("-sn", "--scalarPacketlevelNeuralNetwork", default=False)
parser.add_argument("-d", "--dimensionalPacketlevel", default=True)
parser.add_argument("-dn", "--dimensionalPacketlevelNeuralNetwork", default=True)
parser.add_argument("-b", "--dimensionalSymbollevel", default=True)
parser.add_argument("inifile")
args = parser.parse_args()

if args.scalarPacketlevel :
    os.system(f"opp_runall -b 100 inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevel")

if args.scalarPacketlevelNeuralNetwork :
    os.system(f"opp_runall -b 100 inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevelNeuralNetwork")

if args.dimensionalPacketlevel :
    os.system(f"opp_runall -b 100 inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevel")

if args.dimensionalPacketlevelNeuralNetwork :
    os.system(f"opp_runall -b 100 inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork")

if args.dimensionalSymbollevel :
    os.system(f"opp_runall -b 100 inet -f {args.inifile} -u Cmdenv -c DimensionalSymbollevel")
