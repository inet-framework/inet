#!/usr/bin/python3

import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-s", "--scalarPacketlevel", default=False)
parser.add_argument("-sn", "--scalarPacketlevelNeuralNetwork", default=False)
parser.add_argument("-d", "--dimensionalPacketlevel", default=True)
parser.add_argument("-dn", "--dimensionalPacketlevelNeuralNetwork", default=True)
parser.add_argument("-b", "--dimensionalBitlevel", default=True)
parser.add_argument("-o", "--output", default="/dev/null")
parser.add_argument("inifile")
args = parser.parse_args()

if args.scalarPacketlevel :
    os.system(f"opp_runall inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevel --cmdenv-output-file={args.output}")

if args.scalarPacketlevelNeuralNetwork :
    os.system(f"opp_runall inet -f {args.inifile} -u Cmdenv -c ScalarPacketlevelNeuralNetwork --cmdenv-output-file={args.output}")

if args.dimensionalPacketlevel :
    os.system(f"opp_runall inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevel --cmdenv-output-file={args.output}")

if args.dimensionalPacketlevelNeuralNetwork :
    os.system(f"opp_runall inet -f {args.inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork --cmdenv-output-file={args.output}")

if args.dimensionalBitlevel :
    os.system(f"opp_runall inet -f {args.inifile} -u Cmdenv -c DimensionalBitlevel --cmdenv-output-file={args.output}")
