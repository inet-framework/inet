import numpy as np
import os
import matplotlib.pyplot as plt

#plt.rcParams.update({
#    "text.usetex": True,
#    "font.family": "Times"
#})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/ptb/decrease/cc-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
ptbs = result['ptbs']
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
#reduceTlpSizes = result['reduceTlpSizes']
#reduceTlpSizeOnlyIfs = result['reduceTlpSizeOnlyIfs']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
#pcs = result['pcs']
delays = result['delays']
#msgLengths = ['1400B', '1500B', '3000B', '15000B']
measurements = result['measurements']

#lostPacketThresholds = [1, 2, 5, 10]

#fig, ax = plt.subplots(1, 1, figsize=(7.2, 2.8), dpi=200, tight_layout=True)
fig, ax = plt.subplots(1, 1, sharey=True, figsize=(10.71195, 5), dpi=100, tight_layout=True)

lostPacketThresholdColor = {
	1: "b", 
	2: "g", 
	4: "r", 
	20: "m", 
	1000000: "c", 
	1000001: "gray"
}
#linestyles = ['-', '-.', '--', ':']
linestyles = ['-', '--', ':']

pc = 1
#for ptb in ptbs:
for reducePacketTimeThreshold in reducePacketTimeThresholds:
	for lostPacketThreshold in lostPacketThresholds:
		for timeThreshold in timeThresholds:
		
#				label = 'ptb='+ptb
#				ax.errorbar(delays, measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][0], measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1], errorevery=1, label=label)
			for packetType in ['sentPackets', 'lostPackets']:
				#for ptb in ptbs:
				
				diffs = []
				#negError = []
				for delayIndex in range(len(delays)):
					diffs.append(measurements['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][0][delayIndex] - measurements['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][0][delayIndex])
					#negError.append(-measurements['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1][delayIndex])
					#diffsLost.append(measurements['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0][delayIndex] - measurements['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['lostPackets'][0][delayIndex])
		
				#ax.fill_between(delays, measurements['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['sentPackets'][1], negError, color="lightgray", label="without")
				if packetType == 'sentPackets':
					print('sent packets error')
					print(measurements['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][1])
				label = packetType#+', ptb='+ptb
				ax.errorbar(delays, diffs, measurements['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][1], errorevery=1, label=label)
#					print(measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][0])
#					ax.errorbar(delays, measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][0], measurements[ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][packetType][1], errorevery=1, label=label)
			
# 	ax[reducePacketTimeThresholdIndex].set_xlim([0,53])
# 	#ax[reducePacketTimeThresholdIndex].set_xticks([0, 20, 40])
# 	ax[reducePacketTimeThresholdIndex].set_xticks([0, 10, 20, 30, 40, 50])
# 	#ax[reducePacketTimeThresholdIndex].set_xticks([10, 30, 50], minor=True)
# 	ax[reducePacketTimeThresholdIndex].set_ylim([0,3.2])
# 	ax[reducePacketTimeThresholdIndex].set_yticks([0, 1, 2, 3])
# 	ax[reducePacketTimeThresholdIndex].set_yticks([0.5, 1.5, 2.5], minor=True)
# 	ax[reducePacketTimeThresholdIndex].grid(which='both')
# 	ax[reducePacketTimeThresholdIndex].legend(loc='upper left')
# 	if reducePacketTimeThreshold == -1:
# 		ax[reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
# 	else:
# 		ax[reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
# 	if reducePacketTimeThresholdIndex == 0:
# 		ax[reducePacketTimeThresholdIndex].set_ylabel('PTB Detection Time [s]')	
# 	if reducePacketTimeThresholdIndex == 1:
# 		ax[reducePacketTimeThresholdIndex].set_xlabel('Bottleneck Delay $\it{d}$ [ms]')
# 			
# 	reducePacketTimeThresholdIndex += 1

ax.set_xlim([0,53])
ax.set_xticks([0, 10, 20, 30, 40, 50])
ax.set_xlabel('One-Way Delay [ms]')
#ax.set_ylabel('Sent Packets')	
ax.set_ylabel('Packets [#]')
#ax.set_yscale('log')
ax.grid()
ax.legend()
plot_dir = dirname+"plots/decrease/cc-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_cc-limited_packets.pdf")
plt.show()