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
path = dirname + "../../../results/quic/ptb/decrease/app-limited/"

msgLengths = ['1400', '1500', '3000', 'intuniform(1042,1442)']
ptbs = ['true', 'false']
lostPacketThresholds = [3]
timeThresholds = [-3]
reducePacketTimeThresholds = [-4]
pktIntervals = [1000, 630.9573, 398.1072, 251.1886, 158.4893, 100, 63.0957, 39.8107, 25.1189, 15.8489, 10, 6.3096, 3.9811, 2.5119, 1.5849, 1, 0.631, 0.3981, 0.2512, 0.1585, 0.1, 0.0631, 0.0398]
pcs = [1]
runs = 100
decreasedPmtu = 1300
pmtuDecreaseTime = 2000
delays = [10, 20, 30, 40]

measurements = {}

def getData(delay, msgLength, pktInterval, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc):
	ptbDetectionTimes = []
	sentPackets = []
	lostPackets = []
	transmissionTimes = []
	
	for run in range(runs):
		file = path + 'delay='+str(delay)+'/msgLength='+str(msgLength)+'/pc='+str(pc)+'/pktInterval='+str(pktInterval)+'/ptb='+str(ptb)+'/n='+str(lostPacketThreshold)+'/t='+str(timeThreshold)+'/r='+str(reducePacketTimeThreshold)+'/'+str(run)+'.vec'
		#print(file)
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw, value
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.router2.ipv4.ip' 
	  and vectorName = 'packetDropUndefined:vector(packetBytes)'
)
 and simtimeRaw > """ + str(pmtuDecreaseTime*1000000000) + """
order by simtimeRaw asc
limit 1
""")
		row = c.fetchone()
		timeFirstPacketDropped = row[0]
		timeFirstPacketDroppedWouldArrive = timeFirstPacketDropped
		timeFirstPacketDroppedWouldArrive += delay*1000000000/4 # propagation delay
		timeFirstPacketDroppedWouldArrive += (row[1]+7)*8 * 1000 # transmission delay
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw, value
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.receiver.quic' 
	  and vectorName = 'packetNumberReceived_cid=0:vector'
)
 and simtimeRaw <= """ + str(timeFirstPacketDroppedWouldArrive) + """
order by simtimeRaw desc
limit 1
""")
		row = c.fetchone()
		lastReceivedPacketNumber = row[1]
		firstDroppedPacketNumber = lastReceivedPacketNumber + 1
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw, value
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.sender.quic' 
	  and vectorName = 'packetNumberSent_cid=0:vector'
)
 and value = """ + str(firstDroppedPacketNumber) + """
limit 1
""")
		row = c.fetchone()
		timeFirstPacketTooBigSent = row[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.sender.quic' 
	  and vectorName = 'dplpmtudSearchPmtu_cid=0_pid=0:vector'
)
  and simtimeRaw >= """ + str(timeFirstPacketTooBigSent) + """
order by simtimeRaw asc
limit 1
""")
		row = c.fetchone()
		timePtbDetected = row[0]
		c.close()
		ptbDetectionTime = (timePtbDetected - timeFirstPacketTooBigSent) / 1000000000000
		
		c = conn.cursor()
		c.execute("""	
select count(*)
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
""")
		row = c.fetchone()
		numberOfSentPackets = row[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select count(*)
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
  and simtimeRaw >= """ + str(timeFirstPacketTooBigSent) + """
  and value > """ + str(decreasedPmtu+7) + """
""")
		row = c.fetchone()
		numberOfLostPackets = row[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
order by simtimeRaw asc
limit 1
""")
		row = c.fetchone()
		firstSend = row[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck4_.receiver.app[0]' 
	  and vectorName = 'bytesReceived:vector'
)
order by simtimeRaw desc
limit 1
""")
		row = c.fetchone()
		lastReceive = row[0]
		c.close()
		transmissionTime = (lastReceive - firstSend) / 1000000000000

		conn.close()
		
		ptbDetectionTimes.append(ptbDetectionTime)
		sentPackets.append(numberOfSentPackets)
		lostPackets.append(numberOfLostPackets)
		transmissionTimes.append(transmissionTime)
		
	return ptbDetectionTimes, sentPackets, lostPackets, transmissionTimes

for delay in delays:
	measurements[delay] = {}
	
	for msgLength in msgLengths:
		measurements[delay][msgLength] = {}
	
		for ptb in ptbs:
			measurements[delay][msgLength][ptb] = {}
			for lostPacketThreshold in lostPacketThresholds:
				measurements[delay][msgLength][ptb][lostPacketThreshold] = {}

				for timeThreshold in timeThresholds:
					measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold] = {}

					for pc in pcs:
						measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc] = {}
			
						for reducePacketTimeThreshold in reducePacketTimeThresholds:
							if pc > 0 and reducePacketTimeThreshold != -4:
								continue

							measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold] = {}
							measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'] = [[], []]
							measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'] = [[], []]
							measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'] = [[], []]
							measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'] = [[], []]
			
							for pktInterval in pktIntervals:
								ptbDetectionTimes, sentPackets, lostPackets, transmissionTimes = getData(delay, msgLength, pktInterval, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc)
						
								ptbDetectionTimesMean,ptbDetectionTimesCi = mean_confidence_interval(ptbDetectionTimes)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0].append(ptbDetectionTimesMean)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][1].append(ptbDetectionTimesCi)
						
								sentPacketsMean,sentPacketsCi = mean_confidence_interval(sentPackets)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][0].append(sentPacketsMean)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1].append(sentPacketsCi)
						
								lostPacketsMean,lostPacketsCi = mean_confidence_interval(lostPackets)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0].append(lostPacketsMean)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][1].append(lostPacketsCi)
						
								transmissionTimesMean,transmissionTimesCi = mean_confidence_interval(transmissionTimes)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][0].append(transmissionTimesMean)
								measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][1].append(transmissionTimesCi)

								print("delay="+str(delay)+", msgLength="+str(msgLength)+", pktInterval="+str(pktInterval)+" (s="+str(1000/pktInterval)+"), ptb="+ptb+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+": detection time "+str(ptbDetectionTimesMean)+" s, sentPackets "+str(sentPacketsMean)+", lostPackets "+str(lostPacketsMean)+", transmission time "+str(transmissionTimesMean))

result = {
	'delays': delays,
	'msgLengths': msgLengths,
	'ptbs': ptbs,
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'pcs': pcs,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pktIntervals': pktIntervals,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
