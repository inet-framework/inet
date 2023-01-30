#!/usr/bin/python3

import os
import sys
import time
import pprint
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-sp",  "--scalarPacketlevel", default=False, action="store_true")
parser.add_argument("-spn", "--scalarPacketlevelNeuralNetwork", default=False, action="store_true")
parser.add_argument("-dp",  "--dimensionalPacketlevel", default=False, action="store_true")
parser.add_argument("-dpn", "--dimensionalPacketlevelNeuralNetwork", default=False, action="store_true")
parser.add_argument("-ds",  "--dimensionalSymbollevel", default=False, action="store_true")
parser.add_argument("inifile")
args = parser.parse_args()

times = {}

if args.scalarPacketlevel:
    start = time.perf_counter()
    os.system(f"opp_runall -b 1 -j 8 inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevel")
    times["scalarPacketlevel"] = time.perf_counter() - start

if args.scalarPacketlevelNeuralNetwork:
    start = time.perf_counter()
    os.system(f"opp_runall -b 1 -j 8 inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevelNeuralNetwork")
    times["scalarPacketlevelNeuralNetwork"] = time.perf_counter() - start

if args.dimensionalPacketlevel:
    start = time.perf_counter()
    os.system(f"opp_runall -b 1 -j 8 inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevel")
    times["dimensionalPacketlevel"] = time.perf_counter() - start

if args.dimensionalPacketlevelNeuralNetwork:
    start = time.perf_counter()
    os.system(f"opp_runall -b 1 -j 8 inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork")
    times["dimensionalPacketlevelNeuralNetwork"] = time.perf_counter() - start

if args.dimensionalSymbollevel:
    start = time.perf_counter()
    os.system(f"opp_runall -b 1 -j 8 inet -f {args.inifile} -u Cmdenv -c DimensionalSymbollevel")
    times["dimensionalSymbollevel"] = time.perf_counter() - start

pprint.pprint(times)