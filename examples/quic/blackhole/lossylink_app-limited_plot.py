import numpy as np
import os
import matplotlib.pyplot as plt

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/blackhole/lossylink/app-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
delays = result['delays']
packetLossRates = result['packetLossRates']
msgLengths = result['msgLengths']
ccs = result['ccs']
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
pcs = result['pcs']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
pktPerSecs = result['pktPerSecs']
measurements = result['measurements']

fig, ax = plt.subplots(len(msgLengths), len(reducePacketTimeThresholds), sharex='col', sharey='row', figsize=(10.71195, 7.7), dpi=100, tight_layout=True)

lostPacketThresholdColor = {
	1: "b", 
	2: "g", 
	4: "m", 
	16: "orange",
	1000000: "r" 
}
linestyles = ['-', '--', ':']

show = {
	1: {
		0: True
 	},
	2: {
		0: True,
		10: False
	},
	1000000: {
		0: True
	}
}
delay = 10
packetLossRatesShow = 0.02
ccsShow = 'NewReno'

msgLengthIndex = 0
for msgLength in msgLengths:

	reducePacketTimeThresholdIndex = 0
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		negError = []
		for pktIntervalIndex in range(len(pktPerSecs)):
			negError.append(-measurements[delay][packetLossRatesShow][msgLength][ccsShow]['false'][1][pktIntervalIndex])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].fill_between(pktPerSecs, measurements[delay][packetLossRatesShow][msgLength][ccsShow]['false'][1], negError, color="lightgray", label="without")

		for lostPacketThreshold in lostPacketThresholds:
			if lostPacketThreshold not in show:
				continue

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
				if timeThreshold not in show[lostPacketThreshold] or show[lostPacketThreshold][timeThreshold] == False:
					continue
			
				label = ''
				if lostPacketThreshold < 1000000:
					label += r'$n='+str(lostPacketThreshold)
					
					if lostPacketThreshold>1:
						label += r'\wedge t='+str(timeThreshold)
				else:
					label += r'$n=\infty'
					#if timeThreshold == -1: tStr = ", $t=SRTT$"
				label += '$'

				linestyle = linestyles[linestylesIndex]
				color = lostPacketThresholdColor[lostPacketThreshold]
				for pc in pcs:
					if pc > 0:
						continue
						if lostPacketThreshold != 1000000:
							continue
						linestyle = ':'
						color = 'y'
						label = r'$f = '+str(pc)+'$'

					diffs = []
					for pktIntervalIndex in range(len(pktPerSecs)):
						diffs.append(measurements[delay][packetLossRatesShow][msgLength][ccsShow]['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][0][pktIntervalIndex] - measurements[delay][packetLossRatesShow][msgLength][ccsShow]['false'][0][pktIntervalIndex])
					ax[msgLengthIndex][reducePacketTimeThresholdIndex].errorbar(pktPerSecs, diffs, measurements[delay][packetLossRatesShow][msgLength][ccsShow]['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold][1], errorevery=1, capsize=0.0, linestyle=linestyle, color=color, label=label)
		
			linestylesIndex += 1
		
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlim([-10,480])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticks([0, 100, 200, 300, 400])
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticks([100, 300, 500], minor=True)
		
		if msgLength == '1400':
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([-5,55])
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0, 20, 40])
		if msgLength == '1500':
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([-40,150])
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0, 50, 100, 150])
		if msgLength == '3000':
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([-20,40])
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([-20, 0, 20, 40])
		if msgLength == 'intuniform(1042,1442)':
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([-20,150])
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0, 50, 100, 150])

		yticks = ax[msgLengthIndex][reducePacketTimeThresholdIndex].get_yticks()
		yticklabels = []
		for ytick in yticks:
			if ytick < 0:
				yticklabels.append(str(int(ytick)))
			if ytick == 0:
				yticklabels.append('w/o')
			if ytick > 0:
				yticklabels.append('+'+str(int(ytick)))
		#print(yticklabels)
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticklabels(yticklabels)
		
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].grid()
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].grid(which='both')
		
		if msgLengthIndex == 0:
			if reducePacketTimeThreshold == -1:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
		
		#if msgLengthIndex == len(msgLengths)-1:
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set(xlabel='Send Rate [msg/s]')
		
		if reducePacketTimeThresholdIndex == 2:
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].yaxis.set_label_position("right")
			if msgLength == 'intuniform(1042,1442)':
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel('[1042, 1442] B', size='large', rotation=270, labelpad=13)
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel(msgLength+' B', size='large', rotation=270, labelpad=13)
		
		reducePacketTimeThresholdIndex += 1	
	msgLengthIndex += 1

ax[len(msgLengths)-1][1].set(xlabel='Send Rate $\it{s}$ [msg/s]')
ax[0][0].set_ylabel('.', alpha=0)
fig.text(0.013, 0.5, 'Number of Additional Sent Packets', va='center', rotation='vertical')

handles, labels = ax[0][0].get_legend_handles_labels()
fig.legend(handles, labels, loc=[0.41,0.89], ncol=2)

plot_dir = dirname + "plots/lossylink/app-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"lossylink_app-limited.pdf")
plt.show()
