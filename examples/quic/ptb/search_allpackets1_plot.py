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
fill_between_alpha=.2
delay = 20
mtu1 = 1500
ptb1 = 1
# for msgLength in msgLengths:
# 	msgLengthStr = r'$0\ \mathrm{B}$'
# 	if 0 < msgLength and msgLength < 1000:
# 		msgLengthStr = r'$'+str(msgLength)+'\ \mathrm{kB}$'
# 	elif msgLength >= 1000:
# 		msgLengthStr = r'$'+str(int(msgLength/1000))+'\ \mathrm{MB}$'
# 	ptb2 = 1
# 	base = measurements[delay][msgLength][True][ptb1][ptb2][mtu1]
# 	
# 	if msgLength == 1:
# 		continue
# 	label = msgLengthStr
# 	if msgLength == 10:
# 		label = r'$1\ \mathrm{kB}$ or '+msgLengthStr
# 	label += ', w/ PMTUD'
# 	withPmtud = []
# 	ptb2 = 0
# 	for mtu2 in mtus:
# 		withPmtud.append(measurements[delay][msgLength][True][ptb1][ptb2][mtu1][mtu2] - base[mtu2])
# 	ax.plot(mtus, withPmtud, color=colors[msgLength], label=label)

withPmtudMins = []
withPmtudMaxs = []
for mtu2 in mtus:
	withPmtudMin = -1000
	withPmtudMax = -1000
	for msgLength in msgLengths:
		ptb2 = 1
		base = measurements[delay][msgLength][True][ptb1][ptb2][mtu1][mtu2]
		ptb2 = 0
		diff = measurements[delay][msgLength][True][ptb1][ptb2][mtu1][mtu2] - base
		if withPmtudMin == -1000 or diff < withPmtudMin:
			withPmtudMin = diff
		if withPmtudMax == -1000 or diff > withPmtudMax:
			withPmtudMax = diff
	withPmtudMins.append(withPmtudMin)
	withPmtudMaxs.append(withPmtudMax)
ax.fill_between(mtus, withPmtudMins, withPmtudMaxs, color='gray', alpha=fill_between_alpha, label='w/ PMTUD')

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
	#label += ', w/o PMTUD'
	#if msgLength < 10000:
	#	continue
	#if msgLength > 0:
	without = []
	for mtu2 in mtus:
		without.append(measurements[delay][msgLength][False] - base[mtu2])
	ax.plot(mtus, without, linestyle='-', color=colors[msgLength], label=label)

ax.set_yscale('symlog', linthresh=1, linscale=0.5)
ax.set_ylim([-3, 2000])
ax.set_xlim([1275, 1505])
ax.set_yticks([-1, 0, 1, 10, 100, 1000], minor=False)
ax.set_yticklabels(['-1', '0', '1', '10', '100', '1000'])
ax.set_yticks([-3, -2, 2, 3, 4, 5, 6, 7, 8, 9, 20, 30, 40, 50, 60, 70, 80, 90, 200, 300, 400, 500, 600, 700, 800, 900, 2000], minor=True)
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Additional Packets Sent [\#]')
ax.set_xticks([1280, 1300, 1320, 1340, 1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500], minor=False)
ax.set_xticks(mtus, minor=True)
#print(ax.get_ylim())
#ax.set_title('delay = '+str(delay)+' ms')
ax.grid()
ax.legend(loc='upper left', ncol=3)
#ax.legend(loc='upper left')

plot_dir = dirname + "plots/search/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"search_allpackets1_d"+str(delay)+".pdf")
plt.show()

