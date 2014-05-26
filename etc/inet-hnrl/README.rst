inet-hnrl
=========

This directory includes scripts to submit array and batch jobs to a cluster
through the `Sun Grid Engine (SGE)
<http://en.wikipedia.org/wiki/Oracle_Grid_Engine>`_ and do pre- and
post-processing of simulation results.

Simulation run (batch processing)
---------------------------------

:arrayrun.sh:
	Submit a batch job to cluster (Grid Engine) as in the following example:
    
	qsub -t 1:50 -o directoryForStandardOutput -j yes arrayrun.sh iniFile
	configuration

Data analysis (pre- and post-processing)
----------------------------------------

:generateMeansPlotsForAll.pl:
	Plot group means with 95% confidence intervals for batch of simulations

:generateMsgPlotsForAll.pl:
	Plot the number of messages present in the system for batch of simulations
    (to check the warm-up period)
