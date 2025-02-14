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
path = dirname + "../../../results/quic/ptb/search/"

result = np.load(path+"packets.npy", allow_pickle=True).item()
ptbs = result['ptbs']
mtus = result['mtus']
msgLengths = result['msgLengths']
delays = result['delays']
measurements = result['measurements']
msgLengths = [0, 10, 100, 1000, 10000]

fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(6.5, 2.8), dpi=100, tight_layout=True)

#msgLengths = [0, 10, 100, 1000, 10000]
colors = {
	0: 'tab:blue',
	1: 'tab:green',
	10: 'tab:green',
	100: 'tab:red',
	1000: 'tab:purple',
	10000: 'tab:olive'
}
delay = 20
mtu1 = 1500
ptb1 = 1
for msgLength in msgLengths:
	msgLengthStr = r'$0\ \mathrm{B}$'
	if 0 < msgLength and msgLength < 1000:
		msgLengthStr = r'$'+str(msgLength)+'\ \mathrm{kB}$'
	elif msgLength >= 1000:
		msgLengthStr = r'$'+str(int(msgLength/1000))+'\ \mathrm{MB}$'
	ptb2 = 1
	base = measurements[delay][msgLength][True][ptb1][ptb2][mtu1]
	
	if msgLength == 1:
		continue
	label = msgLengthStr
	#if msgLength == 10:
	#	label = r'$1\ \mathrm{kB}$ or '+msgLengthStr
	withPmtud = []
	ptb2 = 0
	for mtu2 in mtus:
		withPmtud.append(measurements[delay][msgLength][True][ptb1][ptb2][mtu1][mtu2] - base[mtu2])
	ax.plot(mtus, withPmtud, color=colors[msgLength], label=label)

#ax.set_yscale('symlog', linthresh=5, linscale=1)
#ax.set_yscale('symlog')
ax.set_xticks([1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500], minor=False)
ax.set_xticks(mtus, minor=True)
ax.set_xlim([1275, 1505])
ax.set_yticks([0, 4, 8, 12, 16, 20])
ax.set_ylim([0, 20])
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Additional Packets Sent [\#]')
#ax.set_title('delay = '+str(delay)+' ms')
ax.grid()
ax.legend(ncol=3)

plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"search_packets1_d"+str(delay)+".pdf")
plt.show()

