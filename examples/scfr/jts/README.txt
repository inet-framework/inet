jts
===

Shortdesc:	'jts' is a set of simulation models and scenarios for the
			investigation of the performance of jitter time-stamp source
			clock frequency	recovery scheme.
Author:		Kyeong Soo (Joseph) Kim (kyeongsoo.kim@gmail.com)
License:	GPL
Requires:	OMNeT++ 4.2.2 or later version with INET-HNRL


The 'jts' is a set of simulation models and scenarios for the investigation
of the performance of jitter time-stamp source clock frequency recovery
scheme.


Features:


Note:
1. Before running a simulation, an SQL database for vectors should be created
   first. For example,
   $ sqlite3 N16_n1_vlow-vector.db < INET-HNRL_ROOT/src/util/database/sqlite/sql/vectors.sql

2. The results from this work is published in [1].


References:
[1] Kyeong Soo Kim, "Asynchronous Source Clock Frequency Recovery through
    Aperiodic Packet Streams," Submitted to IEEE Communications Letters, 2013.

