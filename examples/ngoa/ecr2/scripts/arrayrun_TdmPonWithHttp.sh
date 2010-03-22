#!/bin/bash
#$ -cwd
#$ -V

RUN=$((SGE_TASK_ID-1))
# Run my code for run number $RUN

#cd /users/kks/inet/examples/ngoa/ecr
./run -f TdmPonWithHttp.ini -u Cmdenv -c $1 -r $RUN > /dev/null 2>&1
