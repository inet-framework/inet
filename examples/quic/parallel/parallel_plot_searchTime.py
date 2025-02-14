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
kind = 'searchTime'

fig, ax = plt.subplots(1, 1, figsize=(7, 3.5), dpi=100, tight_layout=True)

linestyle = {
	'parallel': '-',
	'single': '--'
}
msgLengths = [0, 1000, 100000, 10000000]
msgLengths = [0]
modes = [0, 2]
modes = [2]
ptbs = ['false', 'true']
ptbs = ['false']
#ptbs = ['true']
ptb = 'false'

fill_between_alpha=.2
mode = 0
ax.fill_between(pmtus, measurements[mode][ptb][0][kind], measurements[mode][ptb][10000000][kind], color='gray', alpha=fill_between_alpha, label='single probe')
mode = 2
ax.plot(pmtus, measurements[mode][ptb][0][kind], marker='.', label='concurrent probes')

#for msgLength in msgLengths:
#	msgLengthValue = msgLength
#	msgLengthUnit = 'B'
#	if msgLengthValue >= 1000:
# 		msgLengthValue = int(msgLengthValue/1000)
# 		msgLengthUnit = 'KB'
# 	if msgLengthValue >= 1000:
# 		msgLengthValue = int(msgLengthValue/1000)
# 		msgLengthUnit = 'MB'
# 	if msgLengthValue >= 1000:
# 		msgLengthValue = int(msgLengthValue/1000)
# 		msgLengthUnit = 'GB'
		
	#for ptb in ptbs:	
	#	diffs = []
	#	for pmtuIndex in range(len(pmtus)):
	#		diff = measurements[2][ptb][msgLength][kind][pmtuIndex] - measurements[0][ptb][msgLength][kind][pmtuIndex]
	#		diffs.append(diff)
	#	
	#	label = str(msgLengthValue)+' '+msgLengthUnit+', ptb='+ptb
	#	ax.plot(pmtus, diffs, marker='.', label=label)
# 	for ptb in ptbs:
# 		for mode in modes:
# 			label = str(msgLengthValue)+' '+msgLengthUnit+', ptb='+ptb+', mode='+str(mode)
# 			label = 'concurrent probes'
# 			ax.plot(pmtus, measurements[mode][ptb][msgLength][kind], marker='.', label=label)

ax.set_xlim([1276, 1504])
ax.set_xticks(range(1280, 1501, 20), minor=False)
ax.set_xticks(pmtus, minor=True)
#ax.set_ylim([-6, 13])
#ax.set_yticks([-4, -2, 0, 2, 4, 6, 8, 10, 12])

ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Search Time [ms]')
ax.grid()
ax.legend()
plt.show()

fig.savefig("parallel_"+kind+".pdf")
plt.show()
