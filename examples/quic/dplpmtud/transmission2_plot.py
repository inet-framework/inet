import numpy as np
import os
import matplotlib
import matplotlib.pyplot as plt

# change from default Type 3 PostScript fonts to Type 42 (a.k.a. TrueType)
#matplotlib.rcParams['pdf.fonttype'] = 42
#matplotlib.rcParams['ps.fonttype'] = 42

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("transmission2.npy", allow_pickle=True).item()
algs = result["algs"]
pmtus = result["pmtus"]
pktPerSecs = result["pktPerSecs"]
times = result["times"]

fig, ax = plt.subplots(1, 1, figsize=(7.6, 2.8), dpi=200, tight_layout=True)

colors = {
	"OptBinary1284": "y",
	"OptBinary1496": "m",
	"Jump1284": "k",
	"Jump1396": "r"
}
linestyles = {
	"lossDelay": "-", #(0, (5, 3)), 
	"ackDelay": ":" #(0, (1, 1))
}
for alg in algs:
	
	for pmtu in pmtus[alg]:
		#times[algIndex][pmtuIndex][1] = times[algIndex][pmtuIndex][0]
		label = alg + " with PMTU=" + str(pmtu) + "B"
		linestyle = "-" #(0, (5, 3))
		#if pmtu > 1284: linestyle = ":"
		for pktPerSecIndex in range(len(pktPerSecs)):
			times[alg][pmtu][0][pktPerSecIndex] *= 1000
			times[alg][pmtu][1][pktPerSecIndex] *= 1000
		ax.errorbar(pktPerSecs, times[alg][pmtu][0], times[alg][pmtu][1], errorevery=2, capsize=3.0, color=colors[alg+str(pmtu)], linestyle=linestyle, label=label)
		#print(alg+" pktPerSec: "+str(pktPerSecs[0])+" time: "+str(times[algIndex][0]))
		#print(alg+" pktPerSec: "+str(pktPerSecs[len(pktPerSecs)-1])+" time: "+str(times[algIndex][len(pktPerSecs)-1]))
ax.grid()
#plt.grid(axis='x')
gridLine = ax.get_xgridlines()[0]
#xlim = ax.get_xlim()
#print(xlim)
#ax.grid()
#ylim = ax.get_ylim()

#limits = {
#	"OptBinary": [0.12, 0.4920, 0.12, 0.33],
#	"Jump": [0.18, 0.6836, 0.32, 0.9932]
#}

limits = {
	"OptBinary": [240, 809.0625],
}

#maxTime_Up = 2.475
#maxTime_Down = 3.715
#minTime = 1.1
for alg in algs:
	index = 0
	for limit in limits[alg]:
		linestyle = "--"
		if limit == 0.12:
			if index == 0: linestyle = (0, (5, 5))
			else: linestyle = (5, (5, 5))
		
		color = ""
		color = colors[alg+"1284"]
		if index >= 2:
			if alg == "OptBinary": color = colors["OptBinary1496"]
			if alg == "Jump": color = colors["Jump1396"]
		ax.plot([-200, 4200], [limit, limit], linestyle=linestyle, color=color, linewidth=gridLine.get_linewidth())
		index += 1
#ax.plot([-200, 4200], [maxTime_Up, maxTime_Up], "-", color=colors["Up"], linewidth=gridLine.get_linewidth())
#ax.plot([-200, 4200], [minTime, minTime], linestyle=(0, (5, 5)), color=colors["Up"], linewidth=gridLine.get_linewidth())
#ax.plot([-200, 4200], [maxTime_Down, maxTime_Down], "-", color=colors["Down"], linewidth=gridLine.get_linewidth())
#ax.plot([-200, 4200], [minTime, minTime], linestyle=(5, (5, 5)), color=colors["Down"], linewidth=gridLine.get_linewidth())
#ax.set_xlim(xlim)
ax.set_xlim([0.1, 1500])
ax.set_ylim([150, 850])

ax.set(xlabel='Send Rate [msg/s]', ylabel='Search Time [ms]')
ax.legend(loc='upper right')

#ax.set_xscale('symlog', linthresh=0.1, linscale=.05)
ax.set_xscale('log')
#ax.set_yscale('log')

ax.set_xticks([0.1, 1, 10, 100, 1000])
ax.set_xticklabels(['0.1', '1', '10', '100', '1000'])
#ax.set_xticks([0.1, 0.21, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 2, 3, 4, 5, 6, 7, 8, 9, 20, 30, 40, 50, 60, 70, 80, 90, 200, 300, 400, 500, 600, 700, 800, 900, 2000, 3000, 4000], True)

fig.savefig("transmission2.pdf")
plt.show()
	
