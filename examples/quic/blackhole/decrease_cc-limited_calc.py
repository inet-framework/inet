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
path = dirname + "../../../results/quic/blackhole/decrease/cc-limited/"

lostPacketThresholds = [2]
timeThresholds = [
	'(2*$delay)', '(6*$delay+25)', '(10*$delay+2*25)',
	'(4*$delay+25)', '(12*$delay+4*25)', '(20*$delay+7*25)',
	'(8*$delay+3*25)', '(24*$delay+10*25)', '(40*$delay+17*25)'
]
reducePacketTimeThresholds = [-1, -2, -4]
delays = [1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55]
#pcs = [0, 1, 2, 3]
pcs = [0]
runs = 10

measurements = {}

def getPtbDetectionTime(delay, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc):
	ptbDetectionTimes = []
	
	for run in range(runs):
		file = path + 'delay='+str(delay)+'/pc='+str(pc)+'/n='+str(lostPacketThreshold)+'/t='+timeThreshold.replace('$delay', str(delay))+'/r='+str(reducePacketTimeThreshold)+'/'+str(run)+'.vec'
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
and simtimeRaw >= """ + str((1000 + 50*delay) * 1000000000) + """
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
	where moduleName = 'Bottleneck3_.router2.ppp[1].ppp' and vectorName = 'packetDropIncorrectlyReceived:vector(packetBytes)'
) 
and simtimeRaw >= """ + str((1000 + 50*delay) * 1000000000) + """
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
			print("firstDroppedPacketRaw is zero for d="+str(delay)+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", run="+str(run))
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
			print("firstInvalidTimeRaw is zero for d="+str(delay)+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", run="+str(run))
			ptbDetectionTimes.append(float("inf"))
			continue
		
		ptbDetectionTimes.append( (firstInvalidTimeRaw - firstDroppedPacketRaw) / 1000000000000 )
		
	return ptbDetectionTimes


for lostPacketThreshold in lostPacketThresholds:
	measurements[lostPacketThreshold] = {}

	for timeThreshold in timeThresholds:
		measurements[lostPacketThreshold][timeThreshold] = {}

		for pc in pcs:
			measurements[lostPacketThreshold][timeThreshold][pc] = {}
			
			for reducePacketTimeThreshold in reducePacketTimeThresholds:
				if pc > 0 and reducePacketTimeThreshold != -4:
					continue

				measurements[lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold] = [[], []]
			
				for delay in delays:

					firstInvalidTimeMean,firstInvalidTimeCi = mean_confidence_interval(getPtbDetectionTime(delay, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc))
					measurements[lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][0].append(firstInvalidTimeMean)
					measurements[lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][1].append(firstInvalidTimeCi)

					print("delay="+str(delay)+" (RTT="+str(2*delay)+"), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+": detection time "+str(firstInvalidTimeMean)+" s")

result = {
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'pcs': pcs,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'delays': delays,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
