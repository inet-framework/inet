#!/bin/bash


# Initialize variables (customize them if needed).
sum=0


# Total upstream throughput in Gbps
grep "$2" $1 | \
awk 'BEGIN {FS="\t"; i=0} \
	{if ($3 > 0) {i++; sum += $3}} \
	END {print sum/i}'
