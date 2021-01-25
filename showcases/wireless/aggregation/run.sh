#!/bin/bash

inet -u Cmdenv -c NoAggregation &
inet -u Cmdenv -c Aggregation &
inet -u Cmdenv -c VoicePriorityAggregation
