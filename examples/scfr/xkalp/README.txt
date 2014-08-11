xkalp
=====

Shortdesc:	'xkalp' is a set of simulation models and scenarios for the
			investigation of the performance of asynchronous source clock
			frequency recovery schemes.
Author:		Kyeong Soo (Joseph) Kim (kyeongsoo.kim@gmail.com)
License:	GPL
Requires:	OMNeT++ 4.2.2 or later version with INET-HNRL


The 'xkalp' is a set of simulation models and scenarios for the investigation
of the performance of asynchronous source clock frequency recovery schemes.


Features:


Note:
1. Before running a simulation, an SQL database for vectors should be created
   first. For example,
   $ sqlite3 N16_n1_vlow-vector.db < INET-HNRL_ROOT/src/util/database/sqlite/sql/vectors.sql

2. The results from this work is published in [1].


References:
[1] Kyeong Soo Kim, "Asynchronous Source Clock Frequency Recovery through
    Aperiodic Packet Streams," IEEE Communications Letters, vol. 17, no. 7,
    pp. 1455-1458, Jul. 2013.
