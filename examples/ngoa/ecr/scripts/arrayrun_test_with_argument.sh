#!/bin/bash
#$ -cwd
#$ -V

RUN=$((SGE_TASK_ID-1))
# Run my code for run number $RUN

echo "Run = $RUN and 1st argument = $1"
#./run -f EcrReferenceWithHttp.ini -u Cmdenv -c $1 -r $RUN > /dev/null 2>&1
