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
runs = 100
lostPacketThresholds = [1, 2, 3]
timeThresholds = [0, -1]
packetLossRates = [0.01, 0.05]

measurements = []

for lostPacketThresholdIndex in range(len(lostPacketThresholds)):
	lostPacketThreshold = lostPacketThresholds[lostPacketThresholdIndex]
	
	measurements.append([])
	
	for timeThresholdIndex in range(len(timeThresholds)):
		timeThreshold = timeThresholds[timeThresholdIndex]
		if lostPacketThreshold == 1 and timeThreshold != 0:
			continue
			
		measurements[lostPacketThresholdIndex].append([])
		
		packetLossRateIndex = 0
		for packetLossRate in packetLossRates:
			packetLossRate = round(packetLossRate, 2)
		
			if lostPacketThreshold > 1 and packetLossRate != 0.05:
				continue
				
			measurements[lostPacketThresholdIndex][timeThresholdIndex].append([])
			
			measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex].append([])
			measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex].append([])
			
			for pktInterval in pktIntervals:
			
				pmtuInvalidTimes = []
				for run in range(runs):
					file = path + 'packetloss2-pktInterval='+str(pktInterval)+',p='+str(packetLossRate)+',n='+str(lostPacketThreshold)+',t='+str(timeThreshold)+'-'+str(run)+'.vec'
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
					row = c.fetchone()
					pmtuInvalidTime = 0
					if row != None:
						pmtuInvalidTime = row[0]/1000000000000 - 5
						pmtuInvalidTimes.append(pmtuInvalidTime)
					else:
						print("r="+str(1000/pktInterval)+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", p="+str(packetLossRate)+": row none ")
					c.close()

				pmtuInvalidTime,pmtuInvalidTimeCi = mean_confidence_interval(pmtuInvalidTimes)
				#pmtuInvalidTime = np.mean(pmtuInvalidTimes)
				print("r="+str(1000/pktInterval)+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", p="+str(packetLossRate)+": first false positive at "+str(pmtuInvalidTime))
			
				measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex][0].append(pmtuInvalidTime)
				measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex][1].append(pmtuInvalidTimeCi)
				
			packetLossRateIndex += 1
			
pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)
	
result = [
	lostPacketThresholds,
	timeThresholds,
	packetLossRates,
	pktPerSecs,
	measurements
]

np.save(path + "packetloss2", result, allow_pickle=True)
