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
path = dirname + "../../../results/quic/parallel/search/"

parallels = ['true', 'false']
kinds = ['time', 'packets']
pmtus = range(1280, 1501, 4)
runs = 1

def getSearchTime(pmtu, parallel):
	searchTime = 0
	
	for run in range(runs):
		file = path + 'pmtu='+str(pmtu)+'/parallel='+parallel+'/0.sca'
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Parallel.sender.quic' 
  and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last';
""")
		row = c.fetchone()
		searchTime = row[0]*1000
		
		c.close()
		conn.close()
		
	return searchTime

def getNumberOfPackets(pmtu, parallel):
	packets = 0
	
	for run in range(runs):
		file = path + 'pmtu='+str(pmtu)+'/parallel='+parallel+'/0.sca'
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Parallel.sender.quic' 
  and scalarName = 'packetSent:count';
""")
		row = c.fetchone()
		packets = row[0]-1 # probe during BASE
		
		c.close()
		conn.close()
		
	return packets

measurements = {}
for parallel in parallels:
	measurements[parallel] = {}
	for kind in kinds:
		measurements[parallel][kind] = []
	for pmtu in pmtus:
		measurements[parallel]['time'].append(getSearchTime(pmtu, parallel))
		measurements[parallel]['packets'].append(getNumberOfPackets(pmtu, parallel))


result = {
	'parallels': parallels,
	'kinds': kinds,
	'pmtus': pmtus,
	'measurements': measurements
}

np.save("search", result, allow_pickle=True)

