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
path = dirname + "../../../results/quic/ptb/decrease/cc-limited/"

ptbs = ['true', 'false']
lostPacketThresholds = [3]
timeThresholds = [-3]
reducePacketTimeThresholds = [-4]
delays = [1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55]
pcs = [1]
runs = 100
decreasedPmtu = 1300

measurements = {}

def getData(delay, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc):
	ptbDetectionTimes = []
	sentPackets = []
	lostPackets = []
	transmissionTimes = []
	
	pmtuDecreaseTime = 1000 + 50*delay
	for run in range(runs):
		file = path + 'delay='+str(delay)+'/pmtuDecreaseTime='+str(pmtuDecreaseTime)+'/ptb='+str(ptb)+'/pc='+str(pc)+'/n='+str(lostPacketThreshold)+'/t='+str(timeThreshold)+'/r='+str(reducePacketTimeThreshold)+'/'+str(run)+'.vec'
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


for ptb in ptbs:
	measurements[ptb] = {}
	for lostPacketThreshold in lostPacketThresholds:
		measurements[ptb][lostPacketThreshold] = {}

		for timeThreshold in timeThresholds:
			measurements[ptb][lostPacketThreshold][timeThreshold] = {}

			for pc in pcs:
				measurements[ptb][lostPacketThreshold][timeThreshold][pc] = {}
			
				for reducePacketTimeThreshold in reducePacketTimeThresholds:
					if pc > 0 and reducePacketTimeThreshold != -4:
						continue

					measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold] = {}
					measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'] = [[], []]
					measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'] = [[], []]
					measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'] = [[], []]
					measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'] = [[], []]
			
					for delay in delays:
						ptbDetectionTimes, sentPackets, lostPackets, transmissionTimes = getData(delay, ptb, lostPacketThreshold, timeThreshold, reducePacketTimeThreshold, pc)
						
						ptbDetectionTimesMean,ptbDetectionTimesCi = mean_confidence_interval(ptbDetectionTimes)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0].append(ptbDetectionTimesMean)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][1].append(ptbDetectionTimesCi)
						
						sentPacketsMean,sentPacketsCi = mean_confidence_interval(sentPackets)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][0].append(sentPacketsMean)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1].append(sentPacketsCi)
						
						lostPacketsMean,lostPacketsCi = mean_confidence_interval(lostPackets)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0].append(lostPacketsMean)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][1].append(lostPacketsCi)
						
						transmissionTimesMean,transmissionTimesCi = mean_confidence_interval(transmissionTimes)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][0].append(transmissionTimesMean)
						measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][1].append(transmissionTimesCi)

						print("delay="+str(delay)+" (RTT="+str(2*delay)+"), ptb="+ptb+", n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+", r="+str(reducePacketTimeThreshold)+", pc="+str(pc)+": detection time "+str(ptbDetectionTimesMean)+" s, sentPackets "+str(sentPacketsMean)+", lostPackets "+str(lostPacketsMean)+", transmission time "+str(transmissionTimesMean))

result = {
	'ptbs': ptbs,
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'pcs': pcs,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'delays': delays,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
