import numpy as np
import scipy.stats
import sqlite3
import os
from collections import Counter

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

runs = 100
congestionDurations = [50]
reducePacketTimeThresholds = [-1, -2]

measurements = {}

for congestionDuration in congestionDurations:
	measurements[congestionDuration] = {}

	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		measurements[congestionDuration][reducePacketTimeThreshold] = [[], [], []]
	
		maxN = []
		maxT = []
		maxPc = []
		for run in range(runs):
			file = path + 'congestion_bulk-msgLength=50MB,congestionDuration='+str(congestionDuration)+',reducePacketTimeThreshold='+str(reducePacketTimeThreshold)+'-'+str(run)+'.vec'
			#print(file)

			conn = sqlite3.connect(file)
			c = conn.cursor()
			c.execute("""
select max(value)
from vectorData 
where vectorId = (
select vectorId 
from vector 
where moduleName = 'Bottleneck.sender.quic' and vectorName = 'pmtuValidatorNumberOfLostPackets_cid=0_pid=0:vector'
)
""")
			row = c.fetchone()
			if row != None:
				if row[0] != None:
					maxN.append(row[0])
				else:
					maxN.append(0)
			c.close()
			c = conn.cursor()
			c.execute("""
select max(value)
from vectorData 
where vectorId = (
select vectorId 
from vector 
where moduleName = 'Bottleneck.sender.quic' and vectorName = 'pmtuValidatorTimeBetweenLostPackets_cid=0_pid=0:vector'
)
""")
			row = c.fetchone()
			if row != None:
				if row[0] != None:
					maxT.append(row[0])
				else:
					maxT.append(-1)
			c.close()
			c = conn.cursor()
			c.execute("""
select max(value)
from vectorData 
where vectorId = (
select vectorId 
from vector 
where moduleName = 'Bottleneck.sender.quic' and vectorName = 'pmtuValidatorPersistentCongestions_cid=0_pid=0:vector'
)
""")
			row = c.fetchone()
			if row != None:
				if row[0] != None:
					maxPc.append(row[0])
				else:
					maxPc.append(0)
			c.close()
			conn.close()

			print("cd="+str(congestionDuration)+", r="+str(reducePacketTimeThreshold)+": maxN="+str(maxN))
			
			measurements[congestionDuration][reducePacketTimeThreshold][0].append(maxN)
			measurements[congestionDuration][reducePacketTimeThreshold][1].append(maxT)
			measurements[congestionDuration][reducePacketTimeThreshold][2].append(maxPc)

result = {
	'congestionDurations': congestionDurations,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'measurements': measurements
}

np.save(path + "congestion_bulk", result, allow_pickle=True)
