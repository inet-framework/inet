import numpy as np
import os
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load(path+"falsepos1.npy", allow_pickle=True)
iBits = result[0]
sizes = result[1]
delays = result[2]

fig, ax = plt.subplots(1, 1, figsize=(6.4, 2.8), dpi=200, tight_layout=True)

markers = ["x", "+"]
markersizes = [5, 6]

#print("plot for pktPerSecs: "+str(result[0]))
for iBitIndex in [1, 0]: #range(len(iBits)):
	iBit = iBits[iBitIndex]
	print(iBit)
	label = "without I bit"
	if iBit == "true": label = "with I bit"
	ax.plot(sizes, delays[iBitIndex], marker=markers[iBitIndex], markersize=markersizes[iBitIndex], linestyle="None", label=label)

#xlim = ax.get_xlim()
#xticks = ax.get_xticks()
#xticks[0] = 10
#print(xticks)
#ax.set_xticks(xticks)
#ax.set_xlim(xlim)
ax.set_xlim([1,12589])

ax.set(xlabel='Send Data [kB]', ylabel='Delay [ms]')
ax.grid()
ax.legend()

ax.set_xscale('log')
#ax.set_yscale('log')

fig.savefig("falsepos1.png")
plt.show()