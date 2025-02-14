import numpy as np
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("parallel.npy", allow_pickle=True).item()
modes = result['modes']
ptbs = result['ptbs']
msgLengths = result['msgLengths']
pmtus = result['pmtus']
measurements = result['measurements']
kind = 'sentPackets'
ptb = 'false'

fig, ax = plt.subplots(1, 1, figsize=(7.6, 3.5), dpi=100, tight_layout=True)

linestyle = {
	'parallel': '-',
	'single': '--'
}
msgLengths = [1000, 100000, 10000000]

for msgLength in msgLengths:
	msgLengthValue = msgLength
	msgLengthUnit = 'B'
	if msgLengthValue >= 1000:
		msgLengthValue = int(msgLengthValue/1000)
		msgLengthUnit = 'KB'
	if msgLengthValue >= 1000:
		msgLengthValue = int(msgLengthValue/1000)
		msgLengthUnit = 'MB'
	if msgLengthValue >= 1000:
		msgLengthValue = int(msgLengthValue/1000)
		msgLengthUnit = 'GB'
	diffs = []
	for pmtuIndex in range(len(pmtus)):
		diff = measurements[2][ptb][msgLength][kind][pmtuIndex] - measurements[0][ptb][msgLength][kind][pmtuIndex]
		diffs.append(diff)

	ax.plot(pmtus, diffs, marker='.', label=str(msgLengthValue)+' '+msgLengthUnit)

ax.set_xticks(range(1280, 1501, 20))
ax.set_ylim([-6, 13])
ax.set_yticks([-4, -2, 0, 2, 4, 6, 8, 10, 12])
ax.set_yticklabels(['-4', '-2', 'single', '+2', '+4', '+6', '+8', '+10', '+12'])

ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Sent Packets [\#]')
ax.grid()
ax.legend()
plt.show()

fig.savefig("parallel_"+kind+".pdf")
plt.show()
