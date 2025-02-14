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
path = dirname + "../../../results/quic/ptb/decrease/app-limited_blowup/"

msgLengths = ['1400', '1500', '3000', 'intuniform(1042,1442)']
ptbs = ['true', 'false']
lostPacketThresholds = [3]
timeThresholds = [-3]
reducePacketTimeThresholds = [-4]
pktIntervals = [1000, 500, 250, 166.6667, 125, 100, 83.3333, 71.4286, 62.5, 55.5556, 50, 40, 33.3333, 28.5714, 25, 20, 16.6667, 14.2857, 12.5, 11.1111, 10, 8.3333]
pcs = [1]
runs = 100
decreasedPmtu = 1300
pmtuDecreaseTime = 2000
delay = 10

measurements = {}

def getData(msgLength, pktInterval, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc):
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
	where moduleName = 'Bottleneck3_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
order by simtimeRaw asc
""")
		allRows = c.fetchall()
		timeFirstPacketTooBig = 0
		for row in allRows:
			if row[1] <= decreasedPmtu+7:
				continue
			timeFirstPacketTooBig = row[0]
			timeAtRouter1 = timeFirstPacketTooBig
			timeAtRouter1 += delay*1000000000/3 # propagation delay
			timeAtRouter1 += row[1]*8 * 1000 # transmission delay
			if timeAtRouter1 >= pmtuDecreaseTime*1000000000:
				break
		
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.sender.quic' 
	  and vectorName = 'dplpmtudSearchPmtu_cid=0_pid=0:vector'
)
  and simtimeRaw >= """ + str(timeFirstPacketTooBig) + """
order by simtimeRaw asc
limit 1
""")
		ptbDetectionTime = (c.fetchone()[0] - timeFirstPacketTooBig) / 1000000000000
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select count(*)
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
""")
		numberOfSentPackets = c.fetchone()[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select count(*)
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
  and simtimeRaw >= """ + str(timeFirstPacketTooBig) + """
  and value > """ + str(decreasedPmtu+7) + """
""")
		numberOfLostPackets = c.fetchone()[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.sender.ppp[0].ppp' 
	  and vectorName = 'txPk:vector(packetBytes)'
)
order by simtimeRaw asc
limit 1
""")
		firstSend = c.fetchone()[0]
		c.close()
		
		c = conn.cursor()
		c.execute("""	
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck3_.receiver.app[0]' 
	  and vectorName = 'bytesReceived:vector'
)
order by simtimeRaw desc
limit 1
""")
		lastReceive = c.fetchone()[0]
		transmissionTime = (lastReceive - firstSend) / 1000000000000
		c.close()

		conn.close()
		
		ptbDetectionTimes.append(ptbDetectionTime)
		sentPackets.append(numberOfSentPackets)
		lostPackets.append(numberOfLostPackets)
		transmissionTimes.append(transmissionTime)
		
	return ptbDetectionTimes, sentPackets, lostPackets, transmissionTimes


for msgLength in msgLengths:
	measurements[msgLength] = {}
	
	for ptb in ptbs:
		measurements[msgLength][ptb] = {}
		for lostPacketThreshold in lostPacketThresholds:
			measurements[msgLength][ptb][lostPacketThreshold] = {}

			for timeThreshold in timeThresholds:
				measurements[msgLength][ptb][lostPacketThreshold][timeThreshold] = {}

				for pc in pcs:
					measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc] = {}
			
					for reducePacketTimeThreshold in reducePacketTimeThresholds:
						if pc > 0 and reducePacketTimeThreshold != -4:
							continue

						measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold] = {}
						measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'] = [[], []]
						measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'] = [[], []]
						measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'] = [[], []]
						measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'] = [[], []]
			
						for pktInterval in pktIntervals:
							ptbDetectionTimes, sentPackets, lostPackets, transmissionTimes = getData(msgLength, pktInterval, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc)
						
							ptbDetectionTimesMean,ptbDetectionTimesCi = mean_confidence_interval(ptbDetectionTimes)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0].append(ptbDetectionTimesMean)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][1].append(ptbDetectionTimesCi)
						
							sentPacketsMean,sentPacketsCi = mean_confidence_interval(sentPackets)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][0].append(sentPacketsMean)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1].append(sentPacketsCi)
						
							lostPacketsMean,lostPacketsCi = mean_confidence_interval(lostPackets)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0].append(lostPacketsMean)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][1].append(lostPacketsCi)
						
							transmissionTimesMean,transmissionTimesCi = mean_confidence_interval(transmissionTimes)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][0].append(transmissionTimesMean)
							measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][1].append(transmissionTimesCi)

							print("msgLength="+str(msgLength)+", pktInterval="+str(pktInterval)+" (s="+str(1000/pktInterval)+"), ptb="+ptb+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+": detection time "+str(ptbDetectionTimesMean)+" s, sentPackets "+str(sentPacketsMean)+", lostPackets "+str(lostPacketsMean)+", transmission time "+str(transmissionTimesMean))

result = {
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
