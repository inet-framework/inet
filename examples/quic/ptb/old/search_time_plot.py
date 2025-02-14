import numpy as np
import random
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/search/withPMTUD/"

result = np.load(path+"result.npy", allow_pickle=True).item()
ptbs = result['ptbs']
mtus = result['mtus']
msgLengths = result['msgLengths']
delays = result['delays']
measurements = result['measurements']

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
delay = 10
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
			#	ax.fill_between(mtus, measurements[delay][ptb][skip][0], measurements[delay][ptb][skip][msgLength], color=colors[ptb], alpha=fill_between_alpha)
			#ax.plot(mtus, measurements[delay][ptb][skip][0], color=colors[ptb], label='ptb='+str(ptb)+', skip='+skip+', msgLength=0')
			#ax.plot(mtus, measurements[delay][ptb][skip][msgLength], color=colors[ptb], linestyle='--', label='ptb='+str(ptb)+', skip='+skip+', msgLength='+str(int(msgLength/1000))+'MB')
		#for msgLength in msgLengths:
	#		ax.plot(mtus, measurements[delay][ptb][skip][msgLength], label=msgLengthStr+', ptb='+str(ptb))
	
	if msgLength == 1000:
		continue
	label = msgLengthStr
	if msgLength == 10000:
		label = '>=1MB'
	label += ', w/o PTB msg'
	ax.plot(mtus, measurements[delay][0]['true'][msgLength], color=colors[msgLength], label=label)
	
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
	ax.plot(mtus, measurements[delay][1]['true'][msgLength], color=colors[msgLength], linestyle='--', label=label)
		#	ax.plot(x, measurements[delay][ptb][p2][m1], marker=markers[p1][p2], label='p1='+str(p1)+', p2='+str(p2))
		#	ax[i].set_xlabel(mtu1[i])
		#	ax[i].set_xlim(1270, mtu1[i]+10)
		#	ax[i].set_xticks(x)
		#	ax[i].set_xticklabels(x, rotation = 90)

#ax.set_ylim([-20, 570])
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Search Time [ms]')
ax.grid()	
ax.legend()
ax.set_title('delay = '+str(delay)+' ms')

plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"time.pdf")
plt.show()
