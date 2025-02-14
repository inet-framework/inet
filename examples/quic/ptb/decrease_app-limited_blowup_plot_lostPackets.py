import numpy as np
import os
import matplotlib.pyplot as plt
import matplotlib.lines

#plt.rcParams.update({
#    "text.usetex": True,
#    "font.family": "Times"
#})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/decrease/app-limited_blowup/"

result = np.load(path+"result.npy", allow_pickle=True).item()
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
msgLengths = result['msgLengths']
ptbs = result['ptbs']
pcs = result['pcs']
pktIntervals = result['pktIntervals']
measurements = result['measurements']

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

# \textwidth 7.1413in 
# \columnwidth 3.48761in
fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(10.71195, 5), dpi=100, tight_layout=True)

lostPacketThresholdColor = {
	1: "b", 
	2: "g", 
	4: "m", 
	16: "#ff7f0e",
	1000000: "r" 
}
#linestyles = ['-', '-.', '--', ':']
linestyles = ['-', '--', ':']
pc = 1
for msgLength in msgLengths:
	print(msgLength)
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		#print("plot for pktPerSecs: "+str(pktPerSecs))
		for lostPacketThreshold in lostPacketThresholds:

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
	#			for ptb in ptbs:
				bases = measurements[msgLength]['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0]
				diffs = []
				for i in range(len(bases)):
					diffs.append(measurements[msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0][i] - bases[i])
				msgLengthStr = msgLength + ' B'
				if msgLength == 'intuniform(1042,1442)':
					msgLengthStr = '[1042, 1442] B'
				label = msgLengthStr
				ax.errorbar(pktPerSecs, diffs, measurements[msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][1], errorevery=1, label=label)

ax.set_xlim([0,105])
ax.set_xticks([0,20,40,60,80,100])
ax.set_ylim([0,52])
ax.grid()
ax.legend()
ax.set_xlabel('Send Rate [msg/s]')
ax.set_ylabel('Add. Lost Packets')
plot_dir = dirname+"plots/decrease/app-limited_blowup/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_app-limited_blowup_lostPackets.pdf")
plt.show()