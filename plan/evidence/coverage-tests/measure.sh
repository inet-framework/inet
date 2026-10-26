#!/bin/bash
# Usage: measure.sh <tag>
# Runs the jobs of the planned coverage workflow in the coverage container, after "crun.sh build":
# the 4 fingerprint slices and the suites job at the same time, each on its own 4 cores like a
# GitHub runner. Each job has its own profile pool profiles/<tag>-<job>/ and exports
# profiles/<tag>-<job>.lcov; profiles/<tag>-<job>.log holds the test output and the times.
W=/home/levy/workspace/ghci-coverage
T=${1:?tag}
R=/home/runner/work/inet/inet/profiles
cd $W

job() { # <cpuset> <pool> <test name> <command...>
  local cpuset=$1 pool=$2 test_name=$3; shift 3
  CPUSET=$cpuset POOL=$pool tools/crun.sh run bash -c '
    start=$(date +%s)
    "$@" > '$R/$pool'.log 2>&1
    status=$?
    echo "TEST exit=$status seconds=$(( $(date +%s) - start ))" >> '$R/$pool'.log
    echo "POOL bytes=$(du -sb '$R/$pool' | cut -f1) files=$(ls '$R/$pool' | wc -l)" >> '$R/$pool'.log
    start=$(date +%s)
    opp_export_coverage --load @opp -p inet -t '$test_name' -o '$R/$pool'.lcov >> '$R/$pool'.log 2>&1
    echo "EXPORT exit=$? seconds=$(( $(date +%s) - start )) bytes=$(stat -c %s '$R/$pool'.lcov 2>/dev/null)" >> '$R/$pool'.log
  ' bash "$@"
}

for i in 0 1 2 3; do
  job $((4*i))-$((4*i+3)) $T-fingerprint-$i fingerprint \
    opp_run_fingerprint_tests --load @opp -p inet -m coverage --no-build --split-count 4 --split-index $i &
done
(
  for kind in unit module packet queueing protocol; do
    job 16-19 $T-$kind $kind opp_run_${kind}_tests --load @opp -p inet -m coverage
  done
) &
wait
echo "MEASURE-DONE $T"
