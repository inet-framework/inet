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
path = dirname + "../../../results/quic/ptb/search/withPMTUD/"

ptbs = [0, 1]
mtus = range(1280, 1501, 4)
msgLengths = [0, 1, 10, 100, 1000, 10000]
delays = [10, 20, 30, 40]
runs = 1

def getSearchTime(delay, msgLength, ptb1, ptb2, mtu1, mtu2):
	searchTime = 0
	
	for run in range(runs):
		file = path + 'delay='+str(delay)+'/msgLength='+str(msgLength)+'/mtu1='+str(mtu1)+'/mtu2='+str(mtu2)+'/ptb1='+str(ptb1)+'/ptb2='+str(ptb2)+'/'+str(run)
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'.sca?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck4.sender.quic' 
  and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last';
""")
		row = c.fetchone()
		searchTime = row[0]*1000
		
		c.close()
		conn.close()
		
	return searchTime

measurements = {}
for delay in delays:
	measurements[delay] = {}
	for ptb1 in ptbs:
		measurements[delay][ptb1] = {}
		for ptb2 in ptbs:
			measurements[delay][ptb1][ptb2] = {}
			for msgLength in msgLengths:
				measurements[delay][ptb1][ptb2][msgLength] = {}
				for mtu1 in mtus:
					measurements[delay][ptb1][ptb2][msgLength][mtu1] = {}
					for mtu2 in mtus:
						if mtu2 > mtu1:
							break
						measurements[delay][ptb1][ptb2][msgLength][mtu1][mtu2] = getSearchTime(delay, msgLength, ptb1, ptb2, mtu1, mtu2)


result = {
	'delays': delays,
	'msgLengths': msgLengths,
	'ptbs': ptbs,
	'mtus': mtus,
	'measurements': measurements
}

np.save(path + "times", result, allow_pickle=True)

