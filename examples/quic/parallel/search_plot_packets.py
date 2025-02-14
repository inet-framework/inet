import numpy as np
import random
import scipy.stats
import os
import matplotlib.pyplot as plt
import math

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("search.npy", allow_pickle=True).item()
parallels = result['parallels']
pmtus = result['pmtus']
measurements = result['measurements']
kind = 'packets'

fig, ax = plt.subplots(figsize=(7.6, 2.8), dpi=200, tight_layout=True)

diffs = []
for pmtuIndex in range(len(pmtus)):
	pmtu = pmtus[pmtuIndex]
	diff = abs(measurements['false'][kind][pmtuIndex] - measurements['true'][kind][pmtuIndex])
	#print(str(pmtu)+": "+str(diff))
	diffs.append(diff)

linestyle = '-'
ax.plot(pmtus, diffs, linestyle=linestyle, color='y', label='OptBinary')

#for parallel in parallels:	
#	ax.plot(pmtus, measurements[parallel][kind], linestyle=linestyle, color='y', label='parallel='+parallel)
#	linestyle = ':'

#ax[5].legend()
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Additional Sent Packets [\#]')
ax.grid()
ax.legend()
fig.savefig("search_"+kind+".pdf")
plt.show()
