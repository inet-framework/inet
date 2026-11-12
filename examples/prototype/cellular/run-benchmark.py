#!/usr/bin/env python3
#
# Copyright (C) 2026 OpenSim Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Runs configurations of the cellular architecture prototype and reports the wall time, the
# event rate and the counters of the radio medium of each run. Needs the INET and OMNeT++
# environments (the setenv scripts) to be set up.
#

import argparse
import os
import re
import subprocess
import time

MEDIUM_COUNTERS = [
    "transmission count",
    "signal send count",
    "reception computation count",
    "interference computation count",
    "reception decision computation count",
    "listening decision computation count",
]


def read_scalars(sca_path):
    values = {}
    pattern = re.compile(r'^scalar\s+\S*radioMedium\s+"?([^"]+?)"?\s+(\S+)$')
    with open(sca_path) as f:
        for line in f:
            match = pattern.match(line.strip())
            if match:
                values[match.group(1)] = match.group(2)
    return values


def run(config, run_number, mode, sim_time_limit):
    runner = "inet" if mode == "release" else "inet_dbg"
    command = [runner, "-u", "Cmdenv", "-c", config, "-r", str(run_number), "--cmdenv-express-mode=true",
               "--cmdenv-status-frequency=1000s", "--result-dir=results-benchmark"]
    if sim_time_limit:
        command.append("--sim-time-limit=" + sim_time_limit)
    start = time.monotonic()
    result = subprocess.run(command, capture_output=True, text=True)
    wall_time = time.monotonic() - start
    if result.returncode != 0:
        raise RuntimeError("%s run %d failed:\n%s" % (config, run_number, result.stdout[-2000:] + result.stderr[-2000:]))
    events = re.findall(r"Event #(\d+)\s+t=(\S+)", result.stdout)
    num_events, sim_time = (int(events[-1][0]), float(events[-1][1])) if events else (0, 0.0)
    sca_path = os.path.join("results-benchmark", "%s-#%d.sca" % (config, run_number))
    return wall_time, num_events, sim_time, read_scalars(sca_path)


def count_runs(config, mode):
    runner = "inet" if mode == "release" else "inet_dbg"
    result = subprocess.run([runner, "-u", "Cmdenv", "-c", config, "-q", "numruns"], capture_output=True, text=True)
    match = re.search(r"Number of runs:\s*(\d+)", result.stdout)
    return int(match.group(1)) if match else 1


def main():
    parser = argparse.ArgumentParser(description="Benchmark the cellular architecture prototype")
    parser.add_argument("configs", nargs="+", help="configurations of omnetpp.ini to run")
    parser.add_argument("--mode", choices=["release", "debug"], default="release", help="build mode of INET to use")
    parser.add_argument("--sim-time-limit", help="override the simulation time limit, for example 5ms")
    args = parser.parse_args()
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    for config in args.configs:
        for run_number in range(count_runs(config, args.mode)):
            wall_time, num_events, sim_time, counters = run(config, run_number, args.mode, args.sim_time_limit)
            print("%s run %d: %.1f s wall time, %d events, %.4f s simulated, %.1f events/s, %.6f simulated s/s" %
                  (config, run_number, wall_time, num_events, sim_time, num_events / wall_time, sim_time / wall_time))
            for name in MEDIUM_COUNTERS:
                print("    %s: %s" % (name, counters.get(name, "?")))


if __name__ == "__main__":
    main()
