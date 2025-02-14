import numpy as np
import random
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import axes3d

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/search/withPMTUD/"

result = np.load(path+"result.npy", allow_pickle=True).item()
ptbs = result['ptbs']
mtus = result['mtus']
msgLengths = result['msgLengths']
delays = result['delays']
measurements = result['measurements']


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
msgLengths = [0, 10000]
fill_between_alpha=.2
mtu1ss = [
	range(1280, 1350, 4),
	range(1352, 1400, 4),
	range(1400, 1440, 4),
	range(1440, 1472, 4),
	range(1472, 1500, 4)
]
for mtu1s in mtu1ss:
	ratios = []
	for mtu in mtu1s:
		if mtu == 1280 or mtu == 1284:
			continue
		ratios.append( (mtu-1288)/4 + 1 )
	fig, ax = plt.subplots(1, len(ratios), sharex='col', sharey='row', width_ratios=ratios, figsize=(20, 4), dpi=100, tight_layout=True, subplot_kw=dict(frame_on=True))#, gridspec_kw=dict(wspace=0))

	mtu1Index = 0
	for mtu1 in mtu1s:
		mtu2s = []
		times = {}
		for ptb1 in ptbs:
			times[ptb1] = {}
			for ptb2 in ptbs:
				if ptb1 == ptb2: continue
				times[ptb1][ptb2] = {}
				for msgLength in msgLengths:
					times[ptb1][ptb2][msgLength] = []
		mtu2Ticks = []
		for mtu2 in mtus:
			if mtu2 >= mtu1:
				break
			#if (mtu2 % 20) == 0:
			#	mtu2Ticks.append(mtu2)
			if mtu2 == 1280:
				continue
			if ((mtu2-1284) % 16) == 0:
				mtu2Ticks.append(mtu2)
			mtu2s.append(mtu2)
				
			for ptb1 in ptbs:
				for ptb2 in ptbs:
					if ptb1 == ptb2: continue
					for msgLength in msgLengths:
						times[ptb1][ptb2][msgLength].append(measurements[delay][ptb1][ptb2][msgLength][mtu1][mtu2])
	
		if len(mtu2s) > 0:
			for ptb1 in ptbs:
				for ptb2 in ptbs:
					if ptb1 == ptb2: continue
					label = 'ptb1'
					linestyle = '-'
					color='#ff7f0e'
					if ptb2 == 1: 
						label = 'ptb2'
						color='#1f77b4'
						#linestyle = '--'
					marker = ''
					if len(mtu2s) == 1:
						ax[mtu1Index].plot([mtu2s[0], mtu2s[0]], [times[ptb1][ptb2][msgLengths[0]][0], times[ptb1][ptb2][msgLengths[1]][0]], color=color, alpha=fill_between_alpha)						
						ax[mtu1Index].plot(mtu2s, times[ptb1][ptb2][msgLengths[0]], marker='.', color=color, label=label)
					else:
						ax[mtu1Index].plot(mtu2s, times[ptb1][ptb2][msgLengths[0]], linestyle=linestyle, color=color, label=label)
						ax[mtu1Index].fill_between(mtu2s, times[ptb1][ptb2][msgLengths[0]], times[ptb1][ptb2][msgLengths[1]], color=color, alpha=fill_between_alpha)
				
			ax[mtu1Index].set_xlabel(mtu1)
			ax[mtu1Index].set_xlim(1280, mtu1)
			ax[mtu1Index].set_ylim(0, 880)
			#if (mtu1 % 20) == 0:
			#	mtu2Ticks.append(mtu1)
			ax[mtu1Index].set_xticks(mtu2Ticks, minor=False)
			ax[mtu1Index].set_xticks(mtu2s, minor=True)
			#ax[mtu1Index].set_xticklabels(mtu2s, rotation = 90)
			ax[mtu1Index].set_xticklabels(mtu2Ticks, rotation = 90)
			ax[mtu1Index].grid()
			mtu1Index += 1
	ax[len(ratios)-1].legend()
	ax[0].set_ylabel('Search Time [ms]', fontsize='medium')
	fig.text(0.009, 0.12, 'MTU2:', fontsize='medium')
	fig.text(0.009, 0.045, 'MTU1:', fontsize='medium')

	#ax.scatter(mtu1s, mtu2s, times)
	#ax.plot(mtu2s, mtu1s, times)
	#ax.stem(mtu2s, mtu1s, times)

	#ax.set(xlabel='MTU2 [B]', ylabel='MTU1 [B]', zlabel='Time [ms]')
	plot_dir = dirname + "plots/search/"
	if not os.path.isdir(plot_dir):
		os.makedirs(plot_dir)
	fig.savefig(plot_dir+"alltimes_d"+str(delay)+"_m"+str(mtu1s[0])+".pdf")

plt.show()
