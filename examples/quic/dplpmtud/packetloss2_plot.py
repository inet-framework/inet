import os
import numpy as np
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load(path+"packetloss2.npy", allow_pickle=True)
lostPacketThresholds = result[0]
timeThresholds = result[1]
packetLossRates = result[2]
pktPerSecs = result[3]
measurements = result[4]

fig, ax = plt.subplots(1, 1, figsize=(7.2, 2.8), dpi=200, tight_layout=True)

formatLostPacketThresholds = ["b", "g", "r", "c", "m"]
formatTimeThresholds = [(0, (5, 3)), (0, (1, 1))]

for lostPacketThresholdIndex in range(len(lostPacketThresholds)):
	lostPacketThreshold = lostPacketThresholds[lostPacketThresholdIndex]
	
	timeThresholdIndex = 0
	for timeThreshold in timeThresholds:
		if lostPacketThreshold == 1 and timeThreshold != 0:
			continue
			
		packetLossRateIndex = 0
		for packetLossRate in packetLossRates:
			packetLossRate = round(packetLossRate, 2)
			
			if lostPacketThreshold > 1 and packetLossRate != 0.05:
				continue
				
			label = "p=" + str(int(packetLossRate*100)) + "%, n="+str(lostPacketThreshold)
			linestyle = formatTimeThresholds[timeThresholdIndex]
			color = formatLostPacketThresholds[lostPacketThresholdIndex]
			if lostPacketThreshold == 1:			
				if packetLossRate == 0.01:
					#linestyle = ":"
					color="c"
			if lostPacketThreshold > 1: 
				label += ", t="
				if timeThreshold == 0: label += "0"
				if timeThreshold == -1: label += "SRTT"
			
			ax.errorbar(pktPerSecs, measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex][0], measurements[lostPacketThresholdIndex][timeThresholdIndex][packetLossRateIndex][1], errorevery=1, capsize=3.0, linestyle=linestyle, color=color, label=label)

			packetLossRateIndex += 1 
		timeThresholdIndex += 1
#xlim = ax.get_xlim()
#xticks = ax.get_xticks()
#xticks[0] = 10
#print(xticks)
#ax.set_xticks(xticks)
#ax.set_xlim(xlim)
ax.grid()
ax.set(xlabel='Send Rate [msg/s]', ylabel='False Positive Interval [s]')
ax.legend(loc='lower left')

#ax.set_xscale('log')
ax.set_yscale('log')

ax.set_xlim([8,102])
ax.set_xticks([10,20,30,40,50,60,70,80,90,100])

ax.set_yticks([1, 10, 100, 1000])
ax.set_yticklabels(['1', '10', '100', '1000'])

fig.savefig("packetloss2.png")
plt.show()