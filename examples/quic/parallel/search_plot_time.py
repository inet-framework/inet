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
kind = 'time'

fig, ax = plt.subplots(figsize=(7.6, 3.5), dpi=200, tight_layout=True)


for pmtuIndex in range(len(pmtus)):
	pmtu = pmtus[pmtuIndex]
	diff = measurements['false'][kind][pmtuIndex] - measurements['true'][kind][pmtuIndex]
	print(str(pmtu)+": "+str(diff))


for parallel in parallels:
	label = 'OptBinary'
	linestyle = ''
	if parallel == 'true':
		label += ' with'
		linestyle = '-'
	else:
		label += ' without'
		linestyle = ':'
	label += ' concurrent probes'
	ax.plot(pmtus, measurements[parallel][kind], linestyle=linestyle, color='y', marker='.', label=label)

ax.set_xticks(range(1280, 1501, 20))
ax.set_xlabel('PMTU [B]')
ax.set_ylabel('Search Time [ms]')
ax.grid()
ax.legend()
fig.savefig("search_"+kind+".pdf")
plt.show()
