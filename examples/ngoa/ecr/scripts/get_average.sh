#!/bin/bash


# Initialize variables (customize them if needed).
sum=0


# Total upstream throughput in Gbps
grep "$2" $1.sca | \
awk 'BEGIN {FS="\t"; i=0} \
	{if ($3 > 0) {i++; sum += $3}} \
	END {print sum/i}'

# # Total downstream throughput in Gbps
# grep "Total downstream throughput" $1.sca | \
# awk 'BEGIN {FS="\t"; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $3/1e9}' \
# >| $1.down_thr

# # Total throughput in Gbps
# join $1.up_thr $1.down_thr | \
# awk 'BEGIN {FS=" "; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $2+$3}' \
# >| $1.thr

# # Average upstream packet delay in ms
# grep "Total average upstream delay" $1.sca | \
# awk 'BEGIN {FS="\t"; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $3*1e3}' \
# >| $1.up_dly

# # Average downstream packet delay in ms
# grep "Total average downstream delay" $1.sca | \
# awk 'BEGIN {FS="\t"; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $3*1e3}' \
# >| $1.down_dly

# # Total delay in ms
# join $1.up_dly $1.down_dly | join - $1.up_thr | join - $1.down_thr | join - $1.thr | \
# awk 'BEGIN {FS=" "; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", ($2*$4+$3*$5)/$6}' \
# >| $1.dly

# # Total upstream bit loss rate
# grep "Total average upstream bit loss rate" $1.sca | \
# awk 'BEGIN {FS="\t"; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $3}' \
# >| $1.up_blr

# # Total downstream bit loss rate
# grep "Total average downstream bit loss rate" $1.sca | \
# awk 'BEGIN {FS="\t"; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", $3}' \
# >| $1.down_blr

# # Total bit loss rate
# join $1.up_blr $1.down_blr | join - $1.up_thr | join - $1.down_thr | join - $1.thr | \
# awk 'BEGIN {FS=" "; i=0} \
# 	{i++; printf "%.1f", i; print "\t\t", ($2*$4+$3*$5)/$6}' \
# >| $1.blr

# # Downstream Throughput Fairness Index
# awk -v numOnus="$numOnus" 'BEGIN {
# 		FS="\t"
# 		i = 0
# 		j = 0
# 		sum1 = 0
# 		sum2 = 0
# 	}
# 	/Number of downstream bits received/	{
# 		i++
# 		sum1 += $3
# 		sum2 += $3*$3
# 		if (i == numOnus) {
# 			j++
# 			printf "%.1f\t\t%e\n", j, sum1*sum1 / (numOnus*sum2)
# 			sum1 = 0
# 			sum2 = 0
# 			i = 0
# 		}
# 	}' $1.sca >| $1.down_fid

# # Upstream Throughput Fairness Index
# awk -v numOnus="$numOnus" 'BEGIN {
# 		FS="\t"
# 		i = 0
# 		j = 0
# 		sum1 = 0
# 		sum2 = 0
# 	}
# 	/Number of upstream bits received/	{
# 		i++
# 		sum1 += $3
# 		sum2 += $3*$3
# 		if (i == numOnus) {
# 			j++
# 			printf "%.1f\t\t%e\n", j, sum1*sum1 / (numOnus*sum2)
# 			sum1 = 0
# 			sum2 = 0
# 			i = 0
# 		}
# 	}' $1.sca >| $1.up_fid

# #### Packet loss rate
# ###grep 'Packet loss rate\"' $1.sca | \
# ###awk 'BEGIN {FS="\t"; i=0} \
# ###	{i++; printf "%.1f", i*0.5; print "\t\t", $3}' \
# ###>| $1.plr
# ###
# #### VOQ size (octet)
# ###grep "Average VOQ size \[octet\]" $1.sca | \
# ###awk 'BEGIN {FS="\t"; i=0} \
# ###	{i++; printf "%.1f", i*0.5; print "\t\t", $3}' \
# ###>| $1.vqs
