import numpy as np
import random
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/search/withPMTUD/"

result = np.load(path+"result.npy", allow_pickle=True).item()
ptbs = result['ptbs']
mtus = result['mtus']
msgLengths = result['msgLengths']
delays = result['delays']
measurements = result['measurements']
msgLengths = [0, 10, 100, 1000, 10000]

fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(6.5, 2.8), dpi=100, tight_layout=True)

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
delay = 20
mtu1 = 1500
ptb1 = 1
ptb2 = 0
#msgLengths = [0, 1, 10, 100, 1000]
for msgLength in msgLengths:
	msgLengthStr = r'$0\ \mathrm{B}$'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = r'$'+str(msgLength)+'\ \mathrm{kB}$'
	elif msgLength >= 1000:
		msgLengthStr = r'$'+str(int(msgLength/1000))+'\ \mathrm{MB}$'
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
		label = r'$\ge1\ \mathrm{MB}$'
	label += ', w/o PTB msg'
	ax.plot(mtus, measurements[delay][ptb1][ptb2][msgLength][mtu1].values(), color=colors[msgLength], label=label)
	
ptb2 = 1
for msgLength in msgLengths:
	msgLengthStr = r'$0\ \mathrm{B}$'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = r'$'+str(msgLength)+'\ \mathrm{kB}$'
	elif msgLength >= 1000:
		msgLengthStr = str(int(msgLength/1000))+'MB'
		
	if msgLength == 10 or msgLength == 100 or msgLength == 1000:
		continue
	label = msgLengthStr
	if msgLength == 0:
		label = r'$0\ \mathrm{B}$'# or '+msgLengthStr
	if msgLength == 10000:
		label = r'$\ge10\ \mathrm{kB}$'
	label += ', w/ PTB msg'
	ax.plot(mtus, measurements[delay][ptb1][ptb2][msgLength][mtu1].values(), color=colors[msgLength], linestyle='--', label=label)
		#	ax.plot(x, measurements[delay][ptb][p2][m1], marker=markers[p1][p2], label='p1='+str(p1)+', p2='+str(p2))
		#	ax[i].set_xlabel(mtu1[i])
		#	ax[i].set_xlim(1270, mtu1[i]+10)
		#	ax[i].set_xticks(x)
		#	ax[i].set_xticklabels(x, rotation = 90)

ax.set_xticks([1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500], minor=False)
ax.set_xticks(mtus, minor=True)
ax.set_ylim([-50, 1050])
ax.set_xlim([1275, 1505])
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Search Time [ms]')
ax.grid()
ax.legend(ncols=3)
#ax.set_title('delay = '+str(delay)+' ms')

plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"search_times1_d"+str(delay)+".pdf")
plt.show()
