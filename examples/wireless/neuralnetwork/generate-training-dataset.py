#!/usr/bin/python3

import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("configname")
args = parser.parse_args()

p = subprocess.Popen(f"opp_runall -b 1 inet -f generate-training-dataset.ini -u Cmdenv -c {args.configname}", shell=True)

try:
    p.wait()
except:
    p.kill()
