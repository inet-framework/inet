#!/bin/bash
#
# Shell script for generating a plot for group means of a statistic
# (average session delay, average session throughput, mean transfer
# rate, or decodable frame rate) with 95% confidence intervals using
# GNU R.
#
# CREDITS:
# This script is based on Juan-Carlos Maureira's OMNeT++ analysis
# scripts available at:
# http://www-sop.inria.fr/members/Juan-Carlos.Maureira_Bravo/OMNeT/Analysis_Scripts#
# 
# (C) 2010 Kyeong Soo (Joseph) Kim
#

EXPECTED_ARGS=2

# print usage if the number of command-line args is less than 2
if [ $# -ne $EXPECTED_ARGS ]
then
    echo "Usage: `basename $0` data_file statistic_name"
    exit 1
fi

# initialize variables
INPUT_FILE=$1
STATISTIC=$2

RSCRIPT="
library(gplots);
data <- read.table('${INPUT_FILE}', col.names = c('numHosts', 'repetition', 'value'));
pdf(file='${INPUT_FILE}.pdf', width=10, height=10);
attach(data);
plotmeans(value ~ numHosts, xlab='Number of Hosts per ONU', ylab = '${STATISTIC}', n.label=FALSE);
dev.off();
"
echo $RSCRIPT | R --no-save --quiet
