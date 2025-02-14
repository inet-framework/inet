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

#fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(15, 5), dpi=100, tight_layout=True)
fig, ax = plt.subplots(subplot_kw=dict(projection='3d'))

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
ptb1 = 0
ptb2 = 0
msgLength = 0

#for mtu1 in mtus:
mtu1sel = [1300, 1400, 1500]
for mtu1 in mtus:
	mtu1s = []
	mtu2s = []
	times = []
	for mtu2 in mtus:
		if mtu2 >= mtu1:
			break
		if mtu2 == 1280:
			continue
		mtu1s.append(mtu1)
		mtu2s.append(mtu2)
		times.append(measurements[delay][ptb1][ptb2][msgLength][mtu1][mtu2])
	
	ax.plot(mtu2s, mtu1s, times)

ax.set_xlim([1280, 1500])
ax.set_ylim([1280, 1500])
#ax.scatter(mtu1s, mtu2s, times)
#ax.plot(mtu2s, mtu1s, times)
#ax.stem(mtu2s, mtu1s, times)

ax.set(xlabel='MTU2 [B]', ylabel='MTU1 [B]', zlabel='Time [ms]')
plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
#fig.savefig(plot_dir+"time1_d"+str(delay)+".pdf")
plt.show()
