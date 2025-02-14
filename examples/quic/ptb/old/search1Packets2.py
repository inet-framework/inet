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
path = dirname + "../../../results/quic/ptb/search1/"

skipPacketNumber = ['true', 'false']
skipPacketNumber = ['true']
ptbs = [0, 1]
mtus = range(1280, 1501, 4)
msgLengths = [0, 1, 10, 100, 1000, 10000]
runs = 1

def getSentPackets(msgLength, pmtud, ptb=0, mtu=0, skipPacketNumber=False):
	sentPackets = 0
	
	for run in range(runs):
		file = path
		if pmtud:
			file += 'withPMTUD/'
		else:
			file += 'withoutPMTUD/'
		file += 'msgLength='+str(msgLength)+'/'
		if pmtud:
			file += 'mtu='+str(mtu)+'/ptb='+str(ptb)+'/skipPacketNumber='+skipPacketNumber+'/'
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
where moduleName = 'Bottleneck3.sender.ppp[0].ppp' 
  and scalarName = 'txPk:count';
""")
		row = c.fetchone()
		sentPackets = row[0] - 1 # - 1 packets to avoid counting the packet for the base probe
		
		#print("s="+str(pktInterval)+" ("+str(1000/pktInterval)+" msg/s), run="+str(run)+": sentPackets="+str(row[0] - 2))
		c.close()
		conn.close()
		
	return sentPackets

measurements = {}
for msgLength in msgLengths:
	measurements[msgLength] = {}
	if msgLength == 0:
		measurements[msgLength][False] = 0
	else:
		measurements[msgLength][False] = getSentPackets(msgLength, False)
	measurements[msgLength][True] = {}
	
	for ptb in ptbs:
		measurements[msgLength][True][ptb] = {}
		for skip in skipPacketNumber:
			measurements[msgLength][True][ptb][skip] = []
			for mtu in mtus:
				measurements[msgLength][True][ptb][skip].append(getSentPackets(msgLength, True, ptb, mtu, skip))


fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(15, 5), dpi=100, tight_layout=True)

skipPacketNumber = ['true']
#msgLengths = [0, 10, 100, 1000, 10000]
colors = {
	0: 'tab:blue',
	1: 'tab:orange',
	10: 'tab:green',
	100: 'tab:red',
	1000: 'tab:purple',
	10000: 'tab:olive'
}
for msgLength in msgLengths:
	msgLengthStr = '0B'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = str(msgLength)+'KB'
	elif msgLength >= 1000:
		msgLengthStr = str(int(msgLength/1000))+'MB'
	base = measurements[msgLength][True][1]['true']
	
	if msgLength == 1:
		continue
	label = msgLengthStr
	if msgLength == 10:
		label = '1KB or '+msgLengthStr
	label += ', w/ PMTUD, w/o PTB msg'
	#print(msgLength)
	#print(base)
	withPmtud = []
	i = 0
	for mtu in mtus:
		withPmtud.append(measurements[msgLength][True][0]['true'][i] - base[i])
		i += 1
	ax.plot(mtus, withPmtud, color=colors[msgLength], label=label)
	
	
for msgLength in msgLengths:
	msgLengthStr = '0B'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = str(msgLength)+'KB'
	elif msgLength >= 1000:
		msgLengthStr = str(int(msgLength/1000))+'MB'
	base = measurements[msgLength][True][1]['true']
	
	if msgLength == 1:
		continue
	label = msgLengthStr
	if msgLength == 10:
		label = '1KB or '+msgLengthStr
	label += ', w/o PMTUD'
	#if msgLength < 10000:
	#	continue
	#if msgLength > 0:
	without = []
	i = 0
	for mtu in mtus:
		without.append(measurements[msgLength][False] - base[i])
		i += 1
	ax.plot(mtus, without, linestyle='--', color=colors[msgLength], label=label)
	#ax.plot(mtus, withPmtud, label=msgLengthStr+', w/ PMTUD, w/o PTB')
#	for ptb in ptbs:
#		for skip in skipPacketNumber:
			#if ptb == 0:
			#	ax.fill_between(mtus, measurements[ptb][skip][0], measurements[ptb][skip][12], color=colors[ptb], alpha=fill_between_alpha)
			#ax.plot(mtus, measurements[ptb][skip][0], color=colors[ptb], label='ptb='+str(ptb)+', skip='+skip+', msgLength=0')
#			ax.plot(mtus, measurements[msgLength][True][ptb][skip], label='w/ ptb='+str(ptb)+', skip='+skip+', msgLength='+str(msgLength))
		#for msgLength in msgLengths:
		#	ax.plot(mtus, measurements[ptb][skip][msgLength], label='ptb='+str(ptb)+', skip='+skip+', msgLength='+str(msgLength))
		#	ax.plot(x, measurements[ptb][p2][m1], marker=markers[p1][p2], label='p1='+str(p1)+', p2='+str(p2))
		#	ax[i].set_xlabel(mtu1[i])
		#	ax[i].set_xlim(1270, mtu1[i]+10)
		#	ax[i].set_xticks(x)
		#	ax[i].set_xticklabels(x, rotation = 90)

#ax.set_yscale('symlog', linthresh=5, linscale=1)
ax.set_yscale('symlog')
#ax.set_ylim([-20, 570])
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Additional Sent Packets')
ax.grid()
ax.legend()

plot_dir = dirname + "plots/search1/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"packets2.pdf")
plt.show()

