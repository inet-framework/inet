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
path = dirname + "../../../results/quic/ptb/decrease/app-limited/"

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
fig, ax = plt.subplots(len(msgLengths), 1, sharex='col', sharey='row', figsize=(10.71195, 10), dpi=100, tight_layout=True)

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
msgLengthIndex = 0
for msgLength in msgLengths:
	
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		#print("plot for pktPerSecs: "+str(pktPerSecs))
		for lostPacketThreshold in lostPacketThresholds:

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
				for ptb in ptbs:
					label = 'ptb='+ptb	
					linestyle = '-'
					if ptb == 'false':
						linestyle = '--'	
					ax[msgLengthIndex].errorbar(pktPerSecs, measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][0], measurements[msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['transmissionTimes'][1], linestyle=linestyle, errorevery=1, label=label)
		
	ax[msgLengthIndex].set_xlim([-2,105])
	ax[msgLengthIndex].set_xticks([0,20,40,60,80,100])
	ax[msgLengthIndex].grid()
	ax[msgLengthIndex].legend()
	ax[msgLengthIndex].set_title(msgLength+' B')
	if msgLengthIndex == len(msgLengths)-1:
		ax[msgLengthIndex].set_xlabel('Send Rate [msg/s]')
	ax[msgLengthIndex].set_ylabel('Transmission Time')
	
# 
# 		if msgLengthIndex == 0:
# 			if reducePacketTimeThreshold == -1:
# 				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
# 			else:
# 				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
# 		
# 		#if msgLengthIndex == len(msgLengths)-1:
# 			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlabel('Send Rate [msg/s]')
# 		
# 		if reducePacketTimeThresholdIndex == 2:
# 			ax[msgLengthIndex][reducePacketTimeThresholdIndex].yaxis.set_label_position("right")
# 			if msgLength == 'intuniform(1042,1442)':
# 				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel('[1042, 1442] B', size='large', rotation=270, labelpad=13)
# 			else:
# 				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel(msgLength+' B', size='large', rotation=270, labelpad=13)
# 			
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].grid()
# 		
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yscale('log')
# 
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlim([-2,105])
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticks([0,20,40,60,80,100])
# 
# 		##ax.set_ylim([0.04,22])
# 		if msgLength == 'intuniform(1042,1442)':
# 			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([0.01,50])
# 		else:
# 			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([0.01,20])
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0.01, 0.1, 1, 10])
# 		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticklabels(['0.01', '0.1', '1', '10'])
# 		
# 		reducePacketTimeThresholdIndex += 1
	msgLengthIndex += 1

#fig.supxlabel("Send Rate [msg/s]", size='medium')
#fig.supylabel("PTB Detection Time [s]", size='medium')
#fig.text(0.5, 0.02, 'Send Rate [msg/s]', ha='center')

#ax[len(msgLengths)-1][1].set_xlabel('Send Rate $\it{s}$ [msg/s]')
#ax[0][0].set_ylabel('.', alpha=0)
#fig.text(0.013, 0.5, 'PTB Detection Time [s]', va='center', rotation='vertical')

#ax[1][0].set(ylabel='1500 B\n Detection Time [s]')

#handles, labels = ax[0][2].get_legend_handles_labels()
#empty = matplotlib.lines.Line2D([], [], color="none")

#handles.insert(1, empty)
#labels.insert(1, "")

#handles.insert(2, empty)
#labels.insert(2, "")
#fig.legend(handles, labels, loc=[0.22,0.718], ncol=3)

plot_dir = dirname+"plots/decrease/app-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_app-limited_transmissionTimes.pdf")
plt.show()