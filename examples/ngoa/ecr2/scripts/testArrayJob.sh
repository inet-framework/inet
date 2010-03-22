#!/bin/sh
#$ -cwd
#$ -V
#$ -e $HOME/tmp/
#$ -o $HOME/tmp/

RUN=$SGE_TASK_ID
# Run my code for run number $RUN

cd /users/kks/inet/examples/ngoa/ecr
DATE=`date`
echo "This is the standard output for run $RUN on $DATE"
echo "Task $RUN has been finshed!"
