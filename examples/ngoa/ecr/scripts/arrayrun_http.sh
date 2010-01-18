#!/bin/bash
#$ -cwd
#$ -V

RUN=$((SGE_TASK_ID-1))
# Run my code for run number $RUN

#cd /users/kks/inet/examples/ngoa/ecr
./run -f http.ini -u Cmdenv -c EcrReferenceWithHttp -r $RUN > /dev/null 2>&1
