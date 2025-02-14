import numpy as np
import random
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt
import math

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

def dian(data, d):
	dians = []
	for j in range(1, d):
		i = (len(data)-1)*j/d
		i_floor = math.floor(i)
		i_ceil = math.ceil(i)
		if i_floor == i_ceil:
			dians.append(data[int(i)])
		else:
			dians.append( int((data[i_ceil] - data[i_floor]) / 2 + data[i_floor]) )
	return dians

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/search/withPMTUD/"

result = np.load(path+"result.npy", allow_pickle=True).item()
ptbs = result['ptbs']
mtus = result['mtus']
msgLengths = result['msgLengths']
delays = result['delays']
measurements = result['measurements']

mtu1s = range(1320, 1500, 40)
ratios = []
for mtu in mtu1s:
	if mtu == 1280 or mtu == 1284:
		continue
	ratios.append( (mtu-1288)/4 + 1 )

#fig, ax = plt.subplots(1, len(ratios), sharex='col', sharey='row', width_ratios=ratios, figsize=(10.71195, 4), dpi=100, tight_layout=True)#, gridspec_kw=dict(left=0.08, wspace=0.1))
fig, ax = plt.subplots(1, len(ratios), sharex='col', sharey='row', width_ratios=ratios, figsize=(6.5, 2.8), dpi=100, tight_layout=True)#, gridspec_kw=dict(left=0.08, wspace=0.1))
#fig.tight_layout(pad=0.1)
#colors = {
#	0: 'b', 
#	1: 'g'
#}
#fill_between_alpha=.2
#skipPacketNumber = ['true']
#msgLength = 10000
delay = 20
msgLengths = [0, 10000]
fill_between_alpha=.2
	
#for mtu1 in mtus:
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
		#if ((mtu2-1288) % 100) == 0:
		#	mtu2Ticks.append(mtu2)
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
				label = 'only R1 sends PTB msg'
				color='#ff7f0e'
				linestyle = '-'
				if ptb2 == 1: 
					label = 'only R2 sends PTB msg'
					color='#1f77b4'
					#linestyle = '--'
				ax[mtu1Index].plot(mtu2s, times[ptb1][ptb2][msgLengths[0]], linestyle=linestyle, color=color, label=label)
				ax[mtu1Index].fill_between(mtu2s, times[ptb1][ptb2][msgLengths[0]], times[ptb1][ptb2][msgLengths[1]], color=color, alpha=fill_between_alpha)
				
		ax[mtu1Index].set_xlabel(mtu1)
		ax[mtu1Index].set_xlim(1280, mtu1)
		ax[mtu1Index].set_ylim(0, 880)
		#if (mtu1 % 20) == 0:
		#	mtu2Ticks.append(mtu1)
#		if len(mtu2s) < 20:
# 			mtu2Ticks = dian(mtu2s, 2)
# 		elif len(mtu2s) < 30:
# 			mtu2Ticks = dian(mtu2s, 3)
# 		elif len(mtu2s) < 40:
# 			mtu2Ticks = dian(mtu2s, 4)
# 		else:
# 			mtu2Ticks = dian(mtu2s, 5)
		if mtu1Index == 0:
			mtu2Ticks = [1292, 1308]
		elif mtu1Index == 1:
			mtu2Ticks = [1292, 1312, 1332, 1352]
		elif mtu1Index == 2:
			mtu2Ticks = [1292, 1316, 1340, 1364, 1388]
		elif mtu1Index == 3:
			mtu2Ticks = [1292, 1320, 1348, 1376, 1404, 1432]
		else:
			mtu2Ticks = [1292, 1328, 1364, 1400, 1436, 1472]
		
		ax[mtu1Index].set_xticks(mtu2Ticks, minor=False)
		ax[mtu1Index].set_xticks(mtu2s, minor=True)
		#ax[mtu1Index].set_xticklabels(mtu2s, rotation = 90)
		ax[mtu1Index].set_xticklabels(mtu2Ticks, rotation = 90)
		ax[mtu1Index].grid()
		mtu1Index += 1
#ax[5].legend()
ax[0].set_ylabel('Search Time [ms]', fontsize='medium')
fig.text(0.018, 0.155, r'$m2$ [B]:', fontsize='medium')
fig.text(0.018, 0.067, r'$m1$ [B]:', fontsize='medium')

handles, labels = ax[0].get_legend_handles_labels()
fig.legend(handles, labels, loc=[0.105, 0.85], ncol=2)

#ax.scatter(mtu1s, mtu2s, times)
#ax.plot(mtu2s, mtu1s, times)
#ax.stem(mtu2s, mtu1s, times)

#ax.set(xlabel='MTU2 [B]', ylabel='MTU1 [B]', zlabel='Time [ms]')
plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
#fig.savefig(plot_dir+"alltimes_d"+str(delay)+"_m"+str(mtu1s[0])+".pdf")
fig.savefig(plot_dir+"search_times_d"+str(delay)+".pdf")
plt.show()
