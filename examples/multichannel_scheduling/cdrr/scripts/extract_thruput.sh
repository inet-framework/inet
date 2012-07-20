#!/bin/bash
for f in $1
do
	grep "host\[.*\]\.mac.*\"bits/sec rcvd" $f | cut -f 3 > `basename $f .sca`.thr
done
