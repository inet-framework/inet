import numpy as np
import os
import matplotlib.pyplot as plt
import matplotlib.lines

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/decrease/app-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
msgLengths = result['msgLengths']
ptbs = result['ptbs']
pcs = result['pcs']
pktIntervals = result['pktIntervals']
delays = result['delays']
measurements = result['measurements']

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSecs.append(1000/pktInterval)

# \textwidth 7.1413in 
# \columnwidth 3.48761in
fig, ax = plt.subplots(1, 1, sharex='col', sharey='row', figsize=(6.5, 2.8), dpi=100, tight_layout=True)

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
delay = 20
for msgLength in msgLengths:
	print(msgLength)
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		#print("plot for pktPerSecs: "+str(pktPerSecs))
		for lostPacketThreshold in lostPacketThresholds:

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
	#			for ptb in ptbs:
				bases = measurements[delay][msgLength]['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0]
				diffs = []
				for i in range(len(bases)):
					diffs.append(measurements[delay][msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0][i] - bases[i])
				msgLengthStr = msgLength + ' B'
				if msgLength == 'intuniform(1042,1442)':
					msgLengthStr = '[1042, 1442] B'
				label = msgLengthStr
				ax.errorbar(pktPerSecs, diffs, measurements[delay][msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][1], errorevery=1, label=label)

ax.set_xscale('log')
ax.set_xlim([0.9,20000])
ax.set_xticks([1, 10, 100, 1000, 10000])
ax.set_xticklabels(['1', '10', '100', '1000', '10000'])
ax.set_yscale('log')
ax.set_ylim([8,600])
ax.set_yticks([10, 20, 50, 100, 200, 500])
ax.set_yticklabels(['10', '20', '50', '100', '200', '500'])
ax.grid()
ax.legend()
ax.set_xlabel('Send Rate [msg/s]')
ax.set_ylabel('Additional Lost Packets [\#]')
#ax.set_title('delay = '+str(delay)+' ms')

plot_dir = dirname+"plots/decrease/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_lostPackets_d"+str(delay)+".pdf")
plt.show()