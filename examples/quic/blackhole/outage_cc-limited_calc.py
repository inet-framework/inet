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
path = dirname + "../../../results/quic/blackhole/outage/cc-limited/"

runs = 10
lostPacketThresholds = [1, 2, 3, 4, 1000000]
timeThresholds = [0, 10, 20, 40, 60, 80, 100]
packetLossRates = [0.5]
reducePacketTimeThresholds = [-1, -2, -4]
pcs=[0, 1]
msgLength = '200Mb'

measurements = {}

def countSentPackets(p, usePmtuValidator='false', n=1, t=0, r=-1, pc=0):
	sentPackets = []
	
	for run in range(runs):
		file = path + 'msgLength='+msgLength+'/usePmtuValidator='+usePmtuValidator+'/p='+str(p)+'/pc='+str(pc)+'/r='+str(r)+'/n='+str(n)+'/t='+str(t)+'/'+str(run)+'.sca'
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
where moduleName = 'Bottleneck2.sender.ppp[0].ppp' 
  and scalarName = 'txPk:count';
""")
		row = c.fetchone()
		sentPackets.append(row[0] - 2) # - 2 packets to avoid counting the packets from the initial DPLPMTUD
		c.close()
		conn.close()
	
	return sentPackets

n_ts = []

firstRound = True
for p in packetLossRates:
	p = round(p, 2)
	measurements[p] = {}

	usePmtuValidator = 'false'
	measurements[p][usePmtuValidator] = [-1, -1]

	sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(p))
	measurements[p][usePmtuValidator][0] = sentPacketsMean
	measurements[p][usePmtuValidator][1] = sentPacketsCi

	print("p="+str(p)+": sentPacketsMean="+str(sentPacketsMean))

	usePmtuValidator = 'true'
	measurements[p][usePmtuValidator] = {}

	for pc in pcs:
		measurements[p][usePmtuValidator][pc] = {}
		
		for r in reducePacketTimeThresholds:
			measurements[p][usePmtuValidator][pc][r] = [[], []]
		
			for n in lostPacketThresholds:
				for t in timeThresholds:						
					if n == 1 and t != 0:
						continue
					if n == 1000000 and t != 0:
						continue
						
					if firstRound:
						n_ts.append(str(n) + '_' + str(t))
					
					sentPacketsMean,sentPacketsCi = mean_confidence_interval(countSentPackets(p, usePmtuValidator, n, t, r, pc))
					measurements[p][usePmtuValidator][pc][r][0].append(sentPacketsMean)
					measurements[p][usePmtuValidator][pc][r][1].append(sentPacketsCi)

					print("m="+msgLength+", p="+str(p)+", pc="+str(pc)+", r="+str(r)+", n="+str(n)+", t="+str(t)+": sentPacketsMean="+str(sentPacketsMean)+", diff="+str(sentPacketsMean-measurements[p]['false'][0]))
					
			firstRound = False

result = {
	'packetLossRates': packetLossRates,
	'msgLength': msgLength,
	'usePmtuValidators': ['false', 'true'],
	'lostPacketThresholds': lostPacketThresholds,
	'timeThresholds': timeThresholds,
	'reducePacketTimeThresholds': reducePacketTimeThresholds,
	'pcs': pcs,
	'n_ts': n_ts,
	'measurements': measurements
}

np.save(path + "result", result, allow_pickle=True)
