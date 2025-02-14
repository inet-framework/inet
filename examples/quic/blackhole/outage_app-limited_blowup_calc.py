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
path = dirname + "../../../results/quic/blackhole/outage/app-limited_blowup/"

pktIntervals = [158.4893, 100, 63.0957, 39.8107, 25.1189, 15.8489, 10, 6.3096, 3.9811, 2.5119, 1.5849, 1, 0.631, 0.3981, 0.2512, 0.1585, 0.1, 0.0631, 0.0398, 0.0251]
runs = 1000
#lostPacketThresholds = [1, 2, 5, 10]
lostPacketThresholds = [1, 2, 4, 1000000]
#timeThresholds = [0, 10, 20, 30, 100]
timeThresholds = [0, 20, 60]
#packetLossRates = [0.01, 0.02]
packetLossRates = [0.5] #[0.1, 0.3, 0.5, 0.7, 0.9]
reducePacketTimeThresholds = [-1,-2,-4]
#numMsgs = 600
outageStops = [3]
pcs=[0]
msgLengths = ['1400', '1500', '3000', 'intuniform(1042,1442)']
delays = [10, 20]

measurements = {}

def countSentPackets(delay, msgLength, pktInterval, p, outageStop, usePmtuValidator='false', n=1, t=0, reducePacketTimeThreshold=-1, pc=0):
	sentPackets = []
	
	for run in range(runs):
		file = path + 'd='+str(delay)+'/msgLength='+msgLength+'/pktInterval='+str(pktInterval)+'/usePmtuValidator='+usePmtuValidator+'/n='+str(n)+'/t='+str(t)+'/p='+str(p)+'/reducePacketTimeThreshold='+str(reducePacketTimeThreshold)+'/outageStop='+str(outageStop)+'/pc='+str(pc)+'/'+str(run)
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
where moduleName = 'Bottleneck3_.sender.ppp[0].ppp' 
  and scalarName = 'txPk:count';
""")
		row = c.fetchone()
		sentPackets.append(row[0] - 2) # - 2 packets to avoid counting the packets from the initial DPLPMTUD
		c.close()
		conn.close()
	
	return sentPackets


for delay in delays:
	measurements[delay] = {}
	for packetLossRate in packetLossRates:
		packetLossRate = round(packetLossRate, 2)
		measurements[delay][packetLossRate] = {}
		
		for msgLength in msgLengths:
			measurements[delay][packetLossRate][msgLength] = {}
		
			for outageStop in outageStops:
				measurements[delay][packetLossRate][msgLength][outageStop] = {}
	
				usePmtuValidator = 'false'
				measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator] = [[], []]
	
				#sentPacketsWithout = []
				for pktInterval in pktIntervals:
					#sentPacketsWithout.append(countSentPackets(pktInterval, packetLossRate))
					sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(delay, msgLength, pktInterval, packetLossRate, outageStop))
					measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][0].append(sentPacketsMean)
					measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][1].append(sentPacketsCi)
	
					print("p="+str(packetLossRate)+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean))
	
				usePmtuValidator = 'true'
				measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator] = {}
	
				for lostPacketThreshold in lostPacketThresholds:
					measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold] = {}
					#measurements[delay][packetLossRate][outageStop][lostPacketThreshold] = {}
	
					for timeThreshold in timeThresholds:
						if lostPacketThreshold == 1 and timeThreshold != 0:
							continue
						if lostPacketThreshold == 4 and timeThreshold != 0:
							continue
						if lostPacketThreshold == 1000000 and timeThreshold != 0:
							continue
						measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold][timeThreshold] = {}
						#measurements[delay][packetLossRate][outageStop][lostPacketThreshold][timeThreshold] = {}
	
						for reducePacketTimeThreshold in reducePacketTimeThresholds:
							measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold] = {}
							#measurements[delay][packetLossRate][outageStop][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold] = [[], []]
	
							for pc in pcs:
								measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc] = [[], []]
							
								pktIntervalIndex = 0
								for pktInterval in pktIntervals:
									#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", p="+str(packetLossRate)+": sentPacketsDiffMean="+str(sentPacketsDiffMean))
									sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(delay, msgLength, pktInterval, packetLossRate, outageStop, usePmtuValidator, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc))
									measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][0].append(sentPacketsMean)
									measurements[delay][packetLossRate][msgLength][outageStop][usePmtuValidator][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][1].append(sentPacketsCi)
				
									print("p="+str(packetLossRate)+", m="+msgLength+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean)+", diff="+str(sentPacketsMean-measurements[delay][packetLossRate][msgLength][outageStop]['false'][0][pktIntervalIndex]))
									pktIntervalIndex += 1

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

result = {
	'delays': delays,
	'packetLossRates': packetLossRates,
	'msgLengths': msgLengths,
	'outageStops': outageStops,
	'usePmtuValidators': ['false', 'true'],
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pcs': pcs,
	'pktPerSecs': pktPerSecs,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
