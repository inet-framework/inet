#!/usr/bin/python3

import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("inifile")
args = parser.parse_args()

os.system(f"opp_runall inet -f {inifile} -u Cmdenv -c ScalarPacketlevel")
os.system(f"opp_runall inet -f {inifile} -u Cmdenv -c ScalarPacketlevelNeuralNetwork")
os.system(f"opp_runall inet -f {inifile} -u Cmdenv -c DimensionalPacketlevel")
os.system(f"opp_runall inet -f {inifile} -u Cmdenv -c DimensionalPacketlevelNeuralNetwork")
os.system(f"opp_runall inet -f {inifile} -u Cmdenv -c DimensionalBitlevel")
