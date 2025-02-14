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
path = dirname + "../../../results/quic/blackhole/decrease/app-limited/"

msgLengths = ['1400', '1500', '3000', 'intuniform(1042,1442)']
pcs = [0, 1]
#pktIntervals = [1000, 500, 250, 166.6667, 125, 100, 83.3333, 71.4286, 62.5, 55.5556, 50, 40, 33.3333, 28.5714, 25, 20, 16.6667, 14.2857, 12.5, 11.1111, 10, 8.3333]
pktIntervals = [1000, 500, 250, 166.6667, 125, 100, 83.3333, 71.4286, 62.5, 55.5556, 50, 45.4545, 41.6667, 38.4615, 35.7143, 33.3333, 31.25, 29.4118, 27.7778, 26.3158, 25, 23.8095, 22.7273, 21.7391, 20.8333, 20, 19.2308, 18.5185, 17.8571, 17.2414, 16.6667, 16.129, 15.625, 15.1515, 14.7059, 14.2857, 13.8889, 13.5135, 13.1579, 12.8205, 12.5, 12.1951, 11.9048, 11.6279, 11.3636, 11.1111, 10.8696, 10.6383, 10.4167, 10.2041, 10, 9.8039, 9.6154, 9.434, 9.2593, 9.0909]
lostPacketThresholds = [1, 2, 4, 16]
timeThresholds = [0, 100, 200, 1000, 2000, 10000]
reducePacketTimeThresholds = [-1,-2,-4]
runs = 1000

measurements = {}

def getPtbDetectionTime(msgLength, pktInterval, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc):
	ptbDetectionTimes = []
	
	for run in range(runs):
		file = path + 'msgLength='+msgLength+'/pc='+str(pc)+'/pktInterval='+str(pktInterval)+'/n='+str(lostPacketThreshold)+'/t='+str(timeThreshold)+'/r='+str(reducePacketTimeThreshold)+'/'+str(run)+'.vec'
		#print(file)

		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		
		c = conn.cursor()
		c.execute("""
select simtimeRaw 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.router1.ipv4.ip' and vectorName = 'packetDropUndefined:vector(packetBytes)'
) 
and simtimeRaw >= 2000000000000
and value > 1300 
order by simtimeRaw asc
limit 1
""")
		firstDroppedPacketRaw = 0
		row = c.fetchone()
		if row != None:
			firstDroppedPacketRaw = row[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""
select simtimeRaw 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.router2.ppp[0].ppp' and vectorName = 'packetDropIncorrectlyReceived:vector(packetBytes)'
) 
and simtimeRaw >= 2000000000000
order by simtimeRaw asc
limit 1
""")
		row = c.fetchone()
		if row != None:
			firstIncorrectlySentRaw = row[0] - 10000000000 # - delay
			if firstIncorrectlySentRaw < firstDroppedPacketRaw:
				firstDroppedPacketRaw = firstIncorrectlySentRaw
		c.close()
		
		if firstDroppedPacketRaw == 0:
			print("firstDroppedPacketRaw is zero for m="+msgLength+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+", run="+str(run))
			ptbDetectionTimes.append(0)
			conn.close()
			continue
		
		c = conn.cursor()
		c.execute("""
select simtimeRaw 
from vectorData 
where vectorId = (
select vectorId 
from vector 
where moduleName = 'Bottleneck3_.sender.quic' and vectorName = 'dplpmtudInvalidSignals_cid=0_pid=0:vector'
) and simtimeRaw >= """ + str(firstDroppedPacketRaw) + """
order by simtimeRaw asc
limit 1
""")
		firstInvalidTimeRaw = 0
		row = c.fetchone()
		if row != None:
			firstInvalidTimeRaw = row[0]
		c.close()
		conn.close()
		
		if firstInvalidTimeRaw == 0:
			print("firstInvalidTimeRaw is zero for m="+msgLength+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+", run="+str(run))
			ptbDetectionTimes.append(float("inf"))
			continue
		
		ptbDetectionTimes.append( (firstInvalidTimeRaw - firstDroppedPacketRaw) / 1000000000000 )
		
	return ptbDetectionTimes

for lostPacketThreshold in lostPacketThresholds:
	measurements[lostPacketThreshold] = {}

	for timeThreshold in timeThresholds:
		if lostPacketThreshold == 1 and timeThreshold != 0:
			continue
		if lostPacketThreshold == 2 and timeThreshold != 10000:
			continue
		if lostPacketThreshold == 4 and (timeThreshold != 0 and timeThreshold != 100 and timeThreshold != 200):
			continue
		if lostPacketThreshold == 16 and (timeThreshold != 0 and timeThreshold != 1000 and timeThreshold != 2000):
			continue
		measurements[lostPacketThreshold][timeThreshold] = {}

		for reducePacketTimeThreshold in reducePacketTimeThresholds:
			measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold] = {}			

			for pc in pcs:
				if pc == 0 and timeThreshold == 10000:
					continue
				if pc > 0 and (timeThreshold != 10000):
					continue
				measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc] = {}

				for msgLength in msgLengths:
					measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][msgLength] = [[], []]
			
					for pktInterval in pktIntervals:
						#packetLosses = []
						ptbDetectionTimeMean,ptbDetectionTimeCi = mean_confidence_interval(getPtbDetectionTime(msgLength, pktInterval, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc))
						measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][msgLength][0].append(ptbDetectionTimeMean)
						measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][msgLength][1].append(ptbDetectionTimeCi)

						#print("m="+msgLength+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+": detection time "+str(ptbDetectionTimeMean)+" s")


pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

result = {
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pcs': pcs,
	'msgLengths': msgLengths,
	'pktPerSecs': pktPerSecs,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
