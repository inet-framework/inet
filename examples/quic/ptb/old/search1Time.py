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
path = dirname + "../../../results/quic/ptb/search1/withPMTUD/"

skipPacketNumber = ['true']#, 'false']
ptbs = [0, 1]
mtus = range(1280, 1501, 4)
msgLengths = [0, 1, 10, 100, 1000, 10000]
runs = 1

def getSearchTime(msgLength, ptb, mtu, skipPacketNumber):
	searchTime = 0
	
	for run in range(runs):
		file = path + 'msgLength='+str(msgLength)+'/mtu='+str(mtu)+'/ptb='+str(ptb)+'/skipPacketNumber='+skipPacketNumber+'/'+str(run)
		
		conn = None
		try: 
			conn = sqlite3.connect('file:'+file+'.sca?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck3.sender.quic' 
  and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last';
""")
		row = c.fetchone()
		searchTime = row[0]*1000
		
		c.close()
		conn.close()
		
	return searchTime

measurements = {}
for ptb in ptbs:
	measurements[ptb] = {}
	for skip in skipPacketNumber:
		measurements[ptb][skip] = {}
		for msgLength in msgLengths:
			measurements[ptb][skip][msgLength] = []
			for mtu in mtus:
				measurements[ptb][skip][msgLength].append(getSearchTime(msgLength, ptb, mtu, skip))


fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(15, 5), dpi=100, tight_layout=True)

#colors = {
#	0: 'b', 
#	1: 'g'
#}
#fill_between_alpha=.2
#skipPacketNumber = ['true']
#msgLength = 10000
colors = {
	0: 'tab:blue',
	1: 'tab:orange',
	10: 'tab:green',
	100: 'tab:red',
	1000: 'tab:purple',
	10000: 'tab:olive'
}
markers = {
	0: 's',
	1: '1',
	10: '2',
	100: '3',
	1000: 'X',
	10000: ','
}
#msgLengths = [0, 1, 10, 100, 1000]
for msgLength in msgLengths:
	msgLengthStr = '0B'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = str(msgLength)+'KB'
	elif msgLength >= 1000:
		msgLengthStr = str(int(msgLength/1000))+'MB'
	#for ptb in ptbs:
	#	for skip in skipPacketNumber:
			#if ptb == 0:
			#	ax.fill_between(mtus, measurements[ptb][skip][0], measurements[ptb][skip][msgLength], color=colors[ptb], alpha=fill_between_alpha)
			#ax.plot(mtus, measurements[ptb][skip][0], color=colors[ptb], label='ptb='+str(ptb)+', skip='+skip+', msgLength=0')
			#ax.plot(mtus, measurements[ptb][skip][msgLength], color=colors[ptb], linestyle='--', label='ptb='+str(ptb)+', skip='+skip+', msgLength='+str(int(msgLength/1000))+'MB')
		#for msgLength in msgLengths:
	#		ax.plot(mtus, measurements[ptb][skip][msgLength], label=msgLengthStr+', ptb='+str(ptb))
	
	if msgLength == 1000:
		continue
	label = msgLengthStr
	if msgLength == 10000:
		label = '>=1MB'
	label += ', w/o PTB msg'
	ax.plot(mtus, measurements[0]['true'][msgLength], color=colors[msgLength], label=label)
	
for msgLength in msgLengths:
	msgLengthStr = '0B'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = str(msgLength)+'KB'
	elif msgLength >= 1000:
		msgLengthStr = str(int(msgLength/1000))+'MB'
		
	if msgLength == 0 or msgLength == 10 or msgLength == 100 or msgLength == 1000:
		continue
	label = msgLengthStr
	if msgLength == 1:
		label = '0KB or '+msgLengthStr
	if msgLength == 10000:
		label = '>=10KB'
	label += ', w/ PTB msg'
	ax.plot(mtus, measurements[1]['true'][msgLength], color=colors[msgLength], linestyle='--', label=label)
		#	ax.plot(x, measurements[ptb][p2][m1], marker=markers[p1][p2], label='p1='+str(p1)+', p2='+str(p2))
		#	ax[i].set_xlabel(mtu1[i])
		#	ax[i].set_xlim(1270, mtu1[i]+10)
		#	ax[i].set_xticks(x)
		#	ax[i].set_xticklabels(x, rotation = 90)

#ax.set_ylim([-20, 570])
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Search Time [ms]')
ax.grid()	
ax.legend()

plot_dir = dirname + "plots/search1/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"time.pdf")
plt.show()
plt.show()

