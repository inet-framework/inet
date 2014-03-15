#!/bin/bash
#$ -cwd
#$ -V

RUN=$((SGE_TASK_ID-1))  # Run my code for run number $RUN
EXPECTED_ARGS=2

# print usage if the number of command-line args is less than 2
if [ $# -ne $EXPECTED_ARGS ]
then
    echo "Usage: `basename $0` ini_name config_name"
    exit
fi

#cd /users/kks/inet/examples/traffic_engineering/ecr
ulimit -s 65500
### For maximum performance
#./run -u Cmdenv -f $1 -u Cmdenv -c $2 -r $RUN > /dev/null 2>&1
### For checking error messages only
#./run -u Cmdenv -f $1 -u Cmdenv -c $2 -r $RUN > /dev/null
### For debugging (both stdout and stderr will be captured in log files)
./run -u Cmdenv -f $1 -u Cmdenv -c $2 -r $RUN
