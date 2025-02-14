import numpy as np
import scipy.stats
import sqlite3
import os

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return m, h

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

pktIntervals = [100, 50, 33.33, 25, 20, 16.67, 14.29, 12.5, 11.11, 10]
lostPacketThresholds = [1,2,3,4]
timeThresholds = [0, -1]
runs = 100

measurements = []

for lostPacketThresholdIndex in range(len(lostPacketThresholds)):
	lostPacketThreshold = lostPacketThresholds[lostPacketThresholdIndex]
	
	measurements.append([])
	
	for timeThresholdIndex in range(len(timeThresholds)):
		timeThreshold = timeThresholds[timeThresholdIndex]
		if lostPacketThreshold == 1 and timeThreshold != 0:
			continue
		
		measurements[lostPacketThresholdIndex].append([])
		
		measurements[lostPacketThresholdIndex][timeThresholdIndex].append([])
		measurements[lostPacketThresholdIndex][timeThresholdIndex].append([])
	
		for pktIntervalIndex in range(len(pktIntervals)):
			pktInterval = pktIntervals[pktIntervalIndex]
			
			pmtuInvalidDetectionTimes = []
			packetLosses = []
			for run in range(runs):
				file = path + 'decrease-pktInterval='+str(pktInterval)+',n='+str(lostPacketThreshold)+',t='+str(timeThreshold)+'-'+str(run)+'.vec'
				#print(file)
			
				conn = sqlite3.connect(file)
				c = conn.cursor()
				c.execute("""
select simtimeRaw 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck2.sender.quic' and vectorName = 'dplpmtudInvalidSignals_cid=0_pid=0:vector'
) and simtimeRaw >= 5000000000000
order by simtimeRaw asc
limit 1
""")
				pmtuInvalidDetectionTimeRaw = c.fetchone()[0]
				pmtuInvalidDetectionTime = pmtuInvalidDetectionTimeRaw/1000000000000 - 5
				pmtuInvalidDetectionTimes.append(pmtuInvalidDetectionTime)
				
# 				c.execute("""
# select count(*) 
# from vectorData 
# where vectorId = (
# 	select vectorId 
# 	from vector 
# 	where moduleName = 'Bottleneck2.router1.ipv4.ip' and vectorName = 'packetDropUndefined:vector(packetBytes)'
# ) and 5000000000000 <= simtimeRaw and simtimeRaw < """ + str(pmtuInvalidDetectionTimeRaw))
# 				packetLosses.append(c.fetchone()[0])
				
				c.close()

			pmtuInvalidDetectionTime,pmtuInvalidDetectionTimeCi = mean_confidence_interval(pmtuInvalidDetectionTimes)
			#pmtuInvalidDetectionTime = np.mean(pmtuInvalidDetectionTimes)
			measurements[lostPacketThresholdIndex][timeThresholdIndex][0].append(pmtuInvalidDetectionTime)
			measurements[lostPacketThresholdIndex][timeThresholdIndex][1].append(pmtuInvalidDetectionTimeCi)
			
			#packetLoss = np.mean(packetLosses)
			#result[1][lostPacketThresholdIndex][timeThresholdIndex][1].append(packetLoss)
		
			#print("pktInterval="+str(pktInterval)+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+": detection time "+str(pmtuInvalidDetectionTime)+" s, packet losses "+str(packetLoss))
	

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)
	
result = [
	lostPacketThresholds,
	timeThresholds,
	pktPerSecs,
	measurements
]

np.save(path + "decrease", result, allow_pickle=True)
