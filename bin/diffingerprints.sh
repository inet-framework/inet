#!/bin/sh

# This script compares the fingerprint evolution of two simulation runs.
# It takes two eventlog files and outputs a diff file that can be used to
# find the first event where the fingerprints don't match.
#
# Configure the simulation by adding the following lines to the ini file:
#
# record-eventlog = true
# cmdenv-log-format = "%f %g [%l] "
# fingerprint = 0000-0000

grep "E #" $1 | awk '{print $13}' | uniq > $1.tmp
grep "E #" $2 | awk '{print $13}' | uniq > $2.tmp
diff -y -t --suppress-common-lines $1.tmp $2.tmp > diffingerprints.diff
rm $1.tmp
rm $2.tmp
f1=$(head -n 1 diffingerprints.diff | awk '{print $1}')
f2=$(head -n 1 diffingerprints.diff | awk '{print $3}')
grep -e "$f1" $1 | head -n 1
grep -e "$f2" $2 | head -n 1
