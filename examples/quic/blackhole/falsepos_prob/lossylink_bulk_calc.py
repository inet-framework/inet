import numpy as np
import scipy.stats
import sqlite3
import os
from collections import Counter

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

runs = 100
packetLossRates = [0.01, 0.02]
reducePacketTimeThresholds = [-1,-2]

measurements = {}

for packetLossRate in packetLossRates:
	packetLossRate = round(packetLossRate, 2)
	
	measurements[packetLossRate] = {}

	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		measurements[packetLossRate][reducePacketTimeThreshold] = [[], []]
	
		maxN = []
		maxT = []
		for run in range(runs):
			file = path + 'lossylink_bulk-msgLength=137MB,p='+str(packetLossRate)+',reducePacketTimeThreshold='+str(reducePacketTimeThreshold)+'-'+str(run)+'.vec'
			#print(file)

			conn = sqlite3.connect(file)
			c = conn.cursor()
			c.execute("""
select max(value)
from vectorData 
where vectorId = (
select vectorId 
from vector 
where moduleName = 'Bottleneck2.sender.quic' and vectorName = 'pmtuValidatorNumberOfLostPackets_cid=0_pid=0:vector'
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
where moduleName = 'Bottleneck2.sender.quic' and vectorName = 'pmtuValidatorTimeBetweenLostPackets_cid=0_pid=0:vector'
)
""")
			row = c.fetchone()
			if row != None:
				if row[0] != None:
					maxT.append(row[0])
				else:
					maxT.append(0)
			c.close()
			conn.close()

		print("p="+str(packetLossRate)+", r="+str(reducePacketTimeThreshold)+": maxN="+str(maxN)+", maxT="+str(maxT))
	
		measurements[packetLossRate][reducePacketTimeThreshold][0].append(maxN)
		measurements[packetLossRate][reducePacketTimeThreshold][1].append(maxT)
			

result = {
	'packetLossRates': packetLossRates,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'measurements': measurements
}

np.save(path + "lossylink_bulk", result, allow_pickle=True)
