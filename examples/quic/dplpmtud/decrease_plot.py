import numpy as np
import os
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load(path+"decrease.npy", allow_pickle=True)
lostPacketThresholds = result[0]
timeThresholds = result[1]
pktPerSecs = result[2]
measurements = result[3]

fig, ax = plt.subplots(1, 1, figsize=(7.2, 2.8), dpi=200, tight_layout=True)

formatLostPacketThresholds = ["b", "g", "r", "m"]
formatTimeThresholds = [(0, (5, 3)), (0, (1, 1))]

#print("plot for pktPerSecs: "+str(result[0]))
for lostPacketThresholdIndex in range(len(lostPacketThresholds)):
	lostPacketThreshold = lostPacketThresholds[lostPacketThresholdIndex]
	
	for timeThresholdIndex in range(len(timeThresholds)):
		timeThreshold = timeThresholds[timeThresholdIndex]
		if lostPacketThreshold == 1 and timeThreshold != 0: 
			continue
		
		tStr = ""
		if lostPacketThreshold>1:
			tStr = ", t=0"
			if timeThreshold == -1: tStr = ", t=SRTT"
			
		ax.errorbar(pktPerSecs, measurements[lostPacketThresholdIndex][timeThresholdIndex][0], measurements[lostPacketThresholdIndex][timeThresholdIndex][1], errorevery=1, capsize=3.0, linestyle=formatTimeThresholds[timeThresholdIndex], color=formatLostPacketThresholds[lostPacketThresholdIndex], label="n="+str(lostPacketThreshold)+tStr)
		
		#print("n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+": "+str(np.mean(result[1][lostPacketThresholdIndex][timeThresholdIndex][1]))+" lost packets")

#xlim = ax.get_xlim()
#xticks = ax.get_xticks()
#xticks[0] = 10
#print(xticks)
#ax.set_xticks(xticks)
#ax.set_xlim(xlim)
ax.set_xlim([8,102])
ax.set_xticks([10,20,30,40,50,60,70,80,90,100])

ax.set(xlabel='Send Rate [msg/s]', ylabel='Detection Time [s]')
ax.grid()
ax.legend()

#ax.set_xscale('log')
#ax.set_yscale('log')

fig.savefig("decrease.png")
plt.show()