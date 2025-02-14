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
path = dirname + "../../../results/quic/blackhole/lossylink/app-limited/"

pktIntervals = [1000, 100, 50, 25, 16.6667, 12.5, 10, 8.3333, 7.1429, 6.25, 5.5556, 5, 4.5455, 4.1667, 3.8462, 3.5714, 3.3333, 3.125, 2.9412, 2.7778, 2.6316, 2.5, 2.381, 2.2727, 2.1739, 2.0833, 2, 1.9231]
runs = 1000
lostPacketThresholds = [1, 2, 1000000]
timeThresholds = [0]
packetLossRates = [0.02]
reducePacketTimeThresholds = [-1, -2, -4]
msgLengths = ['1400', '1500', '3000', 'intuniform(1042,1442)']
ccs=["NewReno"]
pcs = [0]
sendDuration = 60000
delays=[10, 20]

measurements = {}

def countSentPackets(delay, msgLength, pktInterval, p, cc, usePmtuValidator='false', n=1, t=0, reducePacketTimeThreshold=-1, pc=0):
	sentPackets = []
	
	for run in range(runs):
		file = path + 'd='+str(delay)+'/msgLength='+str(msgLength)+'/pktInterval='+str(pktInterval)+'/sendDuration='+str(sendDuration)+'/p='+str(p)+'/usePmtuValidator='+usePmtuValidator
		if usePmtuValidator == 'true':
			file += '/pc='+str(pc)+'/n='+str(n)+'/t='+str(t)+'/r='+str(reducePacketTimeThreshold)
		file += '/'+str(run)+'.sca'
		#print(file)
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
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
		
			for cc in ccs:
				measurements[delay][packetLossRate][msgLength][cc] = {}
	
				usePmtuValidator = 'false'
				measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator] = [[], []]
	
				#sentPacketsWithout = []
				for pktInterval in pktIntervals:
					#sentPacketsWithout.append(countSentPackets(pktInterval, packetLossRate))
					sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(delay, msgLength, pktInterval, packetLossRate, cc))
					measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][0].append(sentPacketsMean)
					measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][1].append(sentPacketsCi)
		
					print("p="+str(packetLossRate)+", m="+msgLength+", cc="+cc+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean))
	
				usePmtuValidator = 'true'
				measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator] = {}
	
				for lostPacketThreshold in lostPacketThresholds:
					measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold] = {}
					#measurements[delay][packetLossRate][lostPacketThreshold] = {}

					for timeThreshold in timeThresholds:
						if lostPacketThreshold == 1 and timeThreshold != 0:
							continue
						if lostPacketThreshold == 1000000 and timeThreshold != 0:
							continue
						measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold][timeThreshold] = {}
						#measurements[delay][packetLossRate][lostPacketThreshold][timeThreshold] = {}

						for pc in pcs:
							if pc > 0 and lostPacketThreshold != 1000000:
								continue
							measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold][timeThreshold][pc] = {}
						
							for reducePacketTimeThreshold in reducePacketTimeThresholds:
								measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold] = [[], []]
								#measurements[delay][packetLossRate][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold] = [[], []]
		
								pktIntervalIndex = 0
								for pktInterval in pktIntervals:
									#sentPacketsWith = countSentPackets(pktInterval, packetLossRate, 'true', lostPacketThreshold, timeThreshold, reducePacketTimeThreshold)
									#sentPacketsDiff = []
									#for run in range(runs):
									#	sentPacketsDiff.append(sentPacketsWith[run] - sentPacketsWithout[pktIntervalIndex][run])
									#sentPacketsDiffMean,sentPacketsDiffCi = mean_confidence_interval(sentPacketsDiff)
									#measurements[delay][packetLossRate][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][0].append(sentPacketsDiffMean)
									#measurements[delay][packetLossRate][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][1].append(sentPacketsDiffCi)
					
									#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" Pkt/s), n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", p="+str(packetLossRate)+": sentPacketsDiffMean="+str(sentPacketsDiffMean))
									sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(delay, msgLength, pktInterval, packetLossRate, cc, usePmtuValidator, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc))
									measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][0].append(sentPacketsMean)
									measurements[delay][packetLossRate][msgLength][cc][usePmtuValidator][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][1].append(sentPacketsCi)
					
									print("p="+str(packetLossRate)+", m="+msgLength+", cc="+cc+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s): sentPacketsMean="+str(sentPacketsMean)+", diff="+str(sentPacketsMean-measurements[delay][packetLossRate][msgLength][cc]['false'][0][pktIntervalIndex]))
									pktIntervalIndex += 1

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

result = {
	'delays': delays,
	'packetLossRates': packetLossRates,
	'msgLengths': msgLengths,
	'ccs': ccs,
	'usePmtuValidators': ['false', 'true'],
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'pcs': pcs,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pktPerSecs': pktPerSecs,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
