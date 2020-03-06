#!/usr/bin/python3

import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("configname")
args = parser.parse_args()

os.system(f"opp_runall inet -f generate-training-dataset.ini -u Cmdenv -c {args.configname}")
