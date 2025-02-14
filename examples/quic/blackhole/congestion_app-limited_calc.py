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
path = dirname + "../../../results/quic/blackhole/congestion/app-limited/"

pktIntervals = [1000, 200, 100, 20, 10, 2, 1, 0.2, 0.1, 0.02, 0.01]
pktIntervals = [1000, 200, 100, 20, 10, 2, 1, 0.2, 0.1]
lostPacketThresholds = [1, 2, 4]
timeThresholds = [0, 20, 60]
reducePacketTimeThresholds = [-1, -2, -4]
reducePacketTimeThresholds = [-1]
runs = 10

measurements = {}

def countSentPackets(pktInterval, usePmtuValidator, n=1, t=0, r=-1):
	sentPackets = []
	
	for run in range(runs):
		file = ''
		if usePmtuValidator:
			file = path + 'pktInterval='+str(pktInterval)+'/withPmtuValidator/n='+str(n)+'/t='+str(t)+'/r='+str(r)+'/'+str(run)
		else:
			file = path + 'pktInterval='+str(pktInterval)+'/withoutPmtuValidator/'+str(run)
		#print(file)

		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'.sca?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck.sender.ppp[0].ppp' 
  and scalarName = 'txPk:count';
""")
		row = c.fetchone()
		sentPackets.append(row[0] - 2) # - 2 packets to avoid counting the packets from the initial DPLPMTUD
		
		#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s), run="+str(run)+": sentPackets="+str(row[0] - 2))
		c.close()
		conn.close()
	
	return sentPackets

usePmtuValidator = False
measurements[usePmtuValidator] = [[], []]

#sentPacketsWithout = []
for pktInterval in pktIntervals:
	#sentPacketsWithout.append(countSentPackets(pktInterval, packetLossRate))
	sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(pktInterval, usePmtuValidator))
	measurements[usePmtuValidator][0].append(sentPacketsMean)
	measurements[usePmtuValidator][1].append(sentPacketsCi)

	print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean))

usePmtuValidator = True
measurements[usePmtuValidator] = {}

for lostPacketThreshold in lostPacketThresholds:
	measurements[usePmtuValidator][lostPacketThreshold] = {}

	for timeThreshold in timeThresholds:
		if lostPacketThreshold == 1 and timeThreshold != 0:
			continue
		measurements[usePmtuValidator][lostPacketThreshold][timeThreshold] = {}

		for reducePacketTimeThreshold in reducePacketTimeThresholds:
			measurements[usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold] = [[], []]
			
			pktIntervalIndex = 0
			for pktInterval in pktIntervals:
				#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", p="+str(packetLossRate)+": sentPacketsDiffMean="+str(sentPacketsDiffMean))
				sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(pktInterval, usePmtuValidator, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold))
				measurements[usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][0].append(sentPacketsMean)
				measurements[usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][1].append(sentPacketsCi)

				print("n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean)+", diff="+str(sentPacketsMean-measurements[False][0][pktIntervalIndex])+", err="+str(sentPacketsCi))
				pktIntervalIndex += 1

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

result = {
	'usePmtuValidators': [False, True],
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pktPerSecs': pktPerSecs,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
