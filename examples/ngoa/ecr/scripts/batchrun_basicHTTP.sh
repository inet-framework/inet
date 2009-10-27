#!/bin/sh
#$ -cwd

#RUN=$((SGE_TASK_ID-1))
# Run my code for run number $RUN

#cd /scratch/kks/tools/omnetpp/inet-hnrl/examples/ngoa/ecr
DATE=`date`
echo "=================================================================="
echo "This is the standard output for run $RUN on $DATE"
echo "=================================================================="
./run -f basicHTTP.ini -u Cmdenv -c TdmPonWithHttp -r 0
