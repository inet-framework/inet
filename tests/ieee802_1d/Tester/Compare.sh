#!/bin/bash
ERROR=0
while read B;do
COMP1="$1/results/$B"
COMP2="$1/References/$B"
OUT=`diff -q -N $COMP1 $COMP2` 
if [ $? = 1 ]; then
	ERROR=1
fi
if test "$OUT" == ""
then
	echo "$B......[OK]"
else
	echo "$B......[FAIL]"
	diff $COMP1 $COMP2
	ERROR=1
fi
done
exit $ERROR
