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

ptb1 = [0, 1]
ptb2 = [0, 1]
mtu1 = range(1280, 1501, 20)
msgLength = 12
runs = 1

def getSearchTime(p1, p2, m1, m2):
	searchTime = 0
	
	for run in range(runs):
		file = path + 'msgLength='+str(msgLength)+'/mtu1='+str(m1)+'/mtu2='+str(m2)+'/ptb1='+str(p1)+'/ptb2='+str(p2)+'/'+str(run)
		
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
for p1 in ptb1:
	measurements[p1] = {}
	
	for p2 in ptb2:
		measurements[p1][p2] = {}
		
		for m1 in mtu1:
			measurements[p1][p2][m1] = []
			
			for m2 in range(1280, m1+1, 20):
				measurements[p1][p2][m1].append(getSearchTime(p1, p2, m1, m2))


markers = [
	['v', '<'],
	['^', '>']
]
		

ratios = []
for i in range(len(mtu1)):
	ratios.append( (mtu1[i]-1280)/20 + 1 )
fig, ax = plt.subplots(1, len(mtu1), sharex='col', sharey='row', width_ratios=ratios, figsize=(20, 7), dpi=100, tight_layout=True, subplot_kw=dict(frame_on=True))#, gridspec_kw=dict(wspace=0))

for p1 in ptb1:
	for p2 in ptb2:
		i = 0
		for m1 in mtu1:
			x = []
			for m2 in range(1280, m1+1, 20):
				x.append(m2)
			ax[i].scatter(x, measurements[p1][p2][m1], marker=markers[p1][p2], label='p1='+str(p1)+', p2='+str(p2))
			ax[i].set_xlabel(mtu1[i])
			ax[i].set_xlim(1270, mtu1[i]+10)
			ax[i].set_xticks(x)
			ax[i].set_xticklabels(x, rotation = 90)
			i += 1
			
ax[len(mtu1)-1].legend()
plt.show()

