#!/bin/sh

# add these lines in omnetpp.ini
#
# record-eventlog = true
# cmdenv-log-format = "%f %g [%l] "
# fingerprint = 0000-0000

grep -e "^E #" $1 | awk '{print $3 " " $13}' | awk '!x[$1]++' > $1.tmp
grep -e "^E #" $2 | awk '{print $3 " " $13}' | awk '!x[$1]++' > $2.tmp
diff -U 1 $1.tmp $2.tmp > diffingerprints.diff
head -n 4 diffingerprints.diff | tail -n 1
#rm $1.tmp
#rm $2.tmp
