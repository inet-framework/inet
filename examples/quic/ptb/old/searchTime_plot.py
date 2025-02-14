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
path = dirname + "../../../results/quic/ptb/search/"

result = np.load(path+"result.npy", allow_pickle=True).item()
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
msgLengths = result['msgLengths']
pcs = result['pcs']
pktPerSecs = result['pktPerSecs']
measurements = result['measurements']

# \textwidth 7.1413in 
# \columnwidth 3.48761in
fig, ax = plt.subplots(len(msgLengths), len(reducePacketTimeThresholds), sharex='col', sharey='row', figsize=(10.71195, 7.7), dpi=100, tight_layout=True)

lostPacketThresholdColor = {
	1: "b", 
	2: "g", 
	4: "m", 
	16: "#ff7f0e",
	1000000: "r" 
}
prop_cycle = plt.rcParams['axes.prop_cycle']
print(prop_cycle.by_key()['color'])
#linestyles = ['-', '-.', '--', ':']
linestyles = ['-', '--', ':']

show = {
	1: {
		0: True
 	},
	2: {
		0: False,
		100: False,
		10000: True
	},
	4: {
		0: True,
		100: True, 
		200: True
	},
	16: {
		0: True,
		400: False, 
		1000: True, 
		2000: True
	}
}

msgLengthIndex = 0
for msgLength in msgLengths:
	
	reducePacketTimeThresholdIndex = 0
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		#print("plot for pktPerSecs: "+str(pktPerSecs))
		for lostPacketThreshold in lostPacketThresholds:
			if lostPacketThreshold not in show:
				continue

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
				if timeThreshold not in show[lostPacketThreshold] or show[lostPacketThreshold][timeThreshold] == False:
					continue
	
				for pc in pcs:
					if pc == 0 and timeThreshold == 10000:
						continue
					if pc > 0 and timeThreshold != 10000:
						continue
					
					label = r'$n='+str(lostPacketThreshold)
					if lostPacketThreshold>1:
						label += "\wedge t="
						if timeThreshold < 1000:
							label += str(timeThreshold)
							if timeThreshold > 0:
								label += '\ \mathrm{ms}'
						else:
							label += str(int(timeThreshold/1000))+'\ \mathrm{s}'
						#if timeThreshold == -1: tStr = ", t=SRTT"

						
						label += '\wedge c = '
						if pc == 0:
							label += '\infty'
						else:
							label += str(pc)
					label += '$'
					
	#			if pcShow != 0 and reducePacketTimeThreshold == -4 and lostPacketThreshold == 1 and timeThreshold == 0:
	#				ax[msgLengthIndex][reducePacketTimeThresholdIndex].errorbar(pktPerSecs, measurements[lostPacketThreshold][timeThreshold][pcShow][reducePacketTimeThreshold][msgLength][0], measurements[lostPacketThreshold][timeThreshold][pcShow][reducePacketTimeThreshold][msgLength][1], errorevery=1, linestyle=linestyles[linestylesIndex], color=lostPacketThresholdColor[lostPacketThreshold], label=nStr+tStr)
					#print('n='+str(lostPacketThreshold)+', t='+str(timeThreshold)+', r='+str(-reducePacketTimeThreshold)+'E, pc='+str(pc)+', msgLength='+str(msgLength))
					ax[msgLengthIndex][reducePacketTimeThresholdIndex].errorbar(reduceList(pktPerSecs), reduceList(measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][msgLength][0]), reduceList(measurements[lostPacketThreshold][timeThreshold][reducePacketTimeThreshold][pc][msgLength][1]), errorevery=1, linestyle=linestyles[linestylesIndex], color=lostPacketThresholdColor[lostPacketThreshold], label=label)
				
				#if reducePacketTimeThreshold == -4 and lostPacketThreshold == 1:
				#	ax[msgLengthIndex][reducePacketTimeThresholdIndex].errorbar(pktPerSecs, measurements[16][2000][1][reducePacketTimeThreshold][msgLength][0], measurements[16][2000][1][reducePacketTimeThreshold][msgLength][1], errorevery=1, linestyle='-', color='c', label='$f=1$')

				
			#print("n="+str(lostPacketThreshold)+", t="+str(timeThreshold)+": "+str(np.mean(result[1][lostPacketThresholdIndex][timeThresholdIndex][1]))+" lost packets")
				linestylesIndex += 1
				
		


		if msgLengthIndex == 0:
			if reducePacketTimeThreshold == -1:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
		
		#if msgLengthIndex == len(msgLengths)-1:
			#ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlabel('Send Rate [msg/s]')
		
		if reducePacketTimeThresholdIndex == 2:
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].yaxis.set_label_position("right")
			if msgLength == 'intuniform(1042,1442)':
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel('[1042, 1442] B', size='large', rotation=270, labelpad=13)
			else:
				ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylabel(msgLength+' B', size='large', rotation=270, labelpad=13)
			
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].grid()
		
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yscale('log')

		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xlim([-2,105])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_xticks([0,20,40,60,80,100])

		##ax.set_ylim([0.04,22])
		if msgLength == 'intuniform(1042,1442)':
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([0.01,50])
		else:
			ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_ylim([0.01,20])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticks([0.01, 0.1, 1, 10])
		ax[msgLengthIndex][reducePacketTimeThresholdIndex].set_yticklabels(['0.01', '0.1', '1', '10'])
		
		reducePacketTimeThresholdIndex += 1
	msgLengthIndex += 1

#fig.supxlabel("Send Rate [msg/s]", size='medium')
#fig.supylabel("PTB Detection Time [s]", size='medium')
#fig.text(0.5, 0.02, 'Send Rate [msg/s]', ha='center')
ax[len(msgLengths)-1][1].set_xlabel('Send Rate $\it{s}$ [msg/s]')
ax[0][0].set_ylabel('.', alpha=0)
fig.text(0.013, 0.5, 'PTB Detection Time [s]', va='center', rotation='vertical')

#ax[1][0].set(ylabel='1500 B\n Detection Time [s]')

handles, labels = ax[0][2].get_legend_handles_labels()
empty = matplotlib.lines.Line2D([], [], color="none")
#handles.insert(1, empty)
#labels.insert(1, "")
handles.insert(2, empty)
labels.insert(2, "")
fig.legend(handles, labels, loc=[0.22,0.718], ncol=3)

plot_dir = dirname+"plots/decrease/app-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_app-limited.pdf")
plt.show()