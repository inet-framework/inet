import numpy as np
import os
import matplotlib.pyplot as plt

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/blackhole/congestion/app-limited_blowup/"

result = np.load(path+"result.npy", allow_pickle=True).item()
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
pktPerSecs = result['pktPerSecs']
measurements = result['measurements']

reducePacketTimeThresholds = [-1]
msgLengths = ['[1042, 1442]']

fig, ax = plt.subplots(len(msgLengths), len(reducePacketTimeThresholds), sharex='col', sharey='row', figsize=(10.71195, 7.7), dpi=100, tight_layout=True)
ax = [[ax]]

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
		0: False, #True,
		10: False,
		20: False, #True,
		40: False,
		60: False, #True, 
		80: False,
		100: False
	},
	3: {
		0: False,
		10: False,
		20: False
	},
	4: {
		0: False, #True,
		10: False,
		20: False,
		40: False,
		60: False, 
		80: False,
		100: False
	},
	1000000: {
		0: True
	}
}

msgLengthIndex = 0
for msgLength in msgLengths:

	reducePacketTimeThresholdIndex = 0
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		diffs = []
		negError = []
		for pktIntervalIndex in range(len(pktPerSecs)):
			diffs.append(0)
			negError.append(-measurements[False][1][pktIntervalIndex])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].fill_between(pktPerSecs, measurements[False][1], negError, color="lightgray", label="without")

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
						label += ' \wedge t='+str(timeThreshold)
						if timeThreshold > 0:
							label += '\ \mathrm{ms}'
					label += '$'
				else:
					label += r'$n=\infty$'

				alp = .2
				alp = 1
				if timeThreshold > 0:
					alp = 1
				diffs = []
				#differrs = []
				posError = []
				negError = []
				for pktIntervalIndex in range(len(pktPerSecs)):
					diffs.append(measurements[True][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][0][pktIntervalIndex] - measurements[False][0][pktIntervalIndex])
					posError.append(diffs[pktIntervalIndex]+measurements[True][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][1][pktIntervalIndex])
					negError.append(diffs[pktIntervalIndex]-measurements[True][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][1][pktIntervalIndex])
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].errorbar(pktPerSecs, diffs, measurements[True][lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][1], errorevery=1, capsize=0.0, linestyle=linestyles[linestylesIndex], color=lostPacketThresholdColor[lostPacketThreshold], label=label, zorder=timeThreshold+2, alpha=alp)
				linestylesIndex += 1
			
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xscale('log')
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlim([8,30000])
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticks([10, 100, 1000, 10000])
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticklabels(['10', '100', '1000', '10000'])
	
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([-15,50])
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0, 20, 40])
		#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticklabels(['w/o', '+20', '+40'])

		ax[msgLengthIndex][reducePacketTimeThresholdIndex].grid()
		
		if msgLengthIndex == 0:
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set(title='$r = '+str(-reducePacketTimeThreshold)+' E$')
			if reducePacketTimeThreshold == -1:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
		#if msgLengthIndex == len(msgLengths)-1:
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set(xlabel='Send Rate [msg/s]')
		if reducePacketTimeThresholdIndex == 2:
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].yaxis.set_label_position("right")
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set(ylabel=msgLength+'\nSent Packets')
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set(ylabel=msgLength)
			if msgLength == 'intuniform(1042,1442)':
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel('[1042, 1442] B', size='large', rotation=270, labelpad=13)
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel(msgLength+' B', size='large', rotation=270, labelpad=13)
			
		reducePacketTimeThresholdIndex += 1
	msgLengthIndex += 1

#ax[len(msgLengths)-1][1].set(xlabel='Send Rate $\it{s}$ [msg/s]')
ax[0][0].set_ylabel('.', alpha=0)
fig.text(0.013, 0.5, 'Number of Additional Sent Packets', va='center', rotation='vertical')

handles, labels = ax[0][0].get_legend_handles_labels()
fig.legend(handles, labels, loc=[0.27,0.892], ncol=4)

#fig.supxlabel('Send Rate')
#fig.supylabel('Additional Sent Packets')

plot_dir = dirname + "plots/congestion/app-limited_blowup/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"congestion_app-limited_blowup.pdf")
plt.show()
