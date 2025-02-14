import numpy as np
import random
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt

def mean_confidence_interval(data, confidence=0.95):
	a = 1.0 * np.array(data)
	n = len(a)
	m, se = np.mean(a), scipy.stats.sem(a)
	h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
	return m, h
	
dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/search/"

ptbs = [0, 1]
mtus = range(1280, 1501, 4)
delays = [10, 20, 30, 40]
msgLengths = [0, 1, 10, 100, 1000, 10000]
runs = 1

def getSentPackets(delay, msgLength, pmtud, ptb1=0, ptb2=0, mtu1=0, mtu2=0):
	sentPackets = 0
	
	for run in range(runs):
		#${resultdir}/withPMTUD/delay=${delay}/msgLength=${msgLength}/mtu1=${mtu1}/mtu2=${mtu2}/ptb1=${ptb1}/ptb2=${ptb2}/${repetition}.vec
		file = path
		if pmtud:
			file += 'withPMTUD/'
		else:
			file += 'withoutPMTUD/'
		file += 'delay='+str(delay)+'/msgLength='+str(msgLength)+'/'
		if pmtud:
			file += 'mtu1='+str(mtu1)+'/mtu2='+str(mtu2)+'/ptb1='+str(ptb1)+'/ptb2='+str(ptb2)+'/'
		file += str(run)
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'.sca?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck4.sender.ppp[0].ppp' 
  and scalarName = 'txPk:count';
""")
		row = c.fetchone()
		sentPackets = row[0] - 1 # - 1 packets to avoid counting the packet for the base probe
		
		#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s), run="+str(run)+": sentPackets="+str(row[0] - 2))
		c.close()
		conn.close()
		
	return sentPackets

measurements = {}
for delay in delays:
	measurements[delay] = {}
	for msgLength in msgLengths:
		measurements[delay][msgLength] = {}
		measurements[delay][msgLength][False] = getSentPackets(delay, msgLength, False)
		measurements[delay][msgLength][True] = {}
	
		for ptb1 in ptbs:
			measurements[delay][msgLength][True][ptb1] = {}
			for ptb2 in ptbs:
				measurements[delay][msgLength][True][ptb1][ptb2] = {}
				for mtu1 in mtus:
					measurements[delay][msgLength][True][ptb1][ptb2][mtu1] = {}
					for mtu2 in mtus:
						if mtu2 > mtu1:
							break
						measurements[delay][msgLength][True][ptb1][ptb2][mtu1][mtu2] = getSentPackets(delay, msgLength, True, ptb1, ptb2, mtu1, mtu2)


result = {
	'delays': delays,
	'msgLengths': msgLengths,
	'ptbs': ptbs,
	'mtus': mtus,
	'measurements': measurements
}

np.save(path + "packets", result, allow_pickle=True)


