import os
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

# change from default Type 3 PostScript fonts to Type 42 (a.k.a. TrueType)
#matplotlib.rcParams['pdf.fonttype'] = 42
#matplotlib.rcParams['ps.fonttype'] = 42
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})


dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load("packetloss.npy", allow_pickle=True).item()
algs = result['algs']
plrs = result['plrs']
measurements = result['measurements']

fig, ax = plt.subplots(1, 1, figsize=(7.6, 2.8), dpi=200, tight_layout=True)
#maxProbesAx = ax.twinx()

colors = {
	"OptBinary": "y",
	"Jump": "k"
}

plrs = plrs*100
print(plrs)
for alg in algs:
	label = alg + " with PMTU = 1496 B"
	#for plrIndex in range(len(plrs)):
	#	measurements[alg]['times'][0][plrIndex] *= 1000
	#	measurements[alg]['times'][1][plrIndex] *= 1000
	#ax.errorbar(plrs, measurements[alg]['times'][0], measurements[alg]['times'][1], capsize=3.0, color=colors[alg], label=label)
	ax.errorbar(plrs, measurements[alg]['maxProbeNumbers'][0], measurements[alg]['maxProbeNumbers'][1], capsize=3.0, color=colors[alg], label=label)

#xticks = ax.get_xticks()
#print(xticks)
#ax.set_xticks(np.arange(0, 11, 1))
#ax.grid()
#gridLine = ax.get_xgridlines()[0]
#xlim = ax.get_xlim()
#ylim = ax.get_ylim()
#ax.set_xlim(xlim)
#ax.set_xlim([0,xlim[1]])
#ax[0].set_ylim(lim)
#ylim = maxProbesAx.get_ylim()
#maxProbesAx.set_ylim([ylim[0], 3])

ax.set(xlabel='Packet Loss Rate [\%]', ylabel='Max Probe Packets [\#]')
#ax.legend(loc='upper left')
ax.legend()
#maxProbesAx.set_ylabel("Max Probe Packets [#]")
#maxProbesAx.legend(loc='upper right')

#ax.set_xscale('log')
#ax.set_yscale('log')

#ax.set_ylim([0.9, 2.1])
#ax.set_yticks([0, 0.5, 1, 1.5, 2])

#maxProbesAx.set_ylim([0.92, 3.08])
#maxProbesAx.set_yticks([1, 1.5, 2, 2.5, 3])
#maxProbesAx.set_yticks([1.5, 2.5], True)

ax.grid()

fig.savefig("packetloss_packets.pdf")
plt.show()