import numpy as np
import os
import matplotlib.pyplot as plt

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/blackhole/decrease/cc-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
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
fig, ax = plt.subplots(1, len(reducePacketTimeThresholds), sharey=True, figsize=(10.71195, 2.5), dpi=100, tight_layout=True)

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

# show = {
# 	1: {
# 		0: True
#  	},
# 	2: {
# 		0: True,
# 		10: False, 
# 		20: True,
# 		40: True,
# 		60: False,
# 		80: True,
# 		100: False,
# 		180: False,
# 		400: False,
# 	},
# 	4: {
# 		0: False,
# 		100: False, 
# 		200: False,
# 		400: False
# 	},
# 	20: {
# 		0: True,
# 		10: False, 
# 		20: True,
# 		40: True,
# 		60: False,
# 		80: True,
# 		100: False,
# 		180: False,
# 		400: False,
# 	}
# }
# reducePacketTimeThresholdShow = -1
# show = {
# 	1: {
# 		0: True
#  	},
# 	2: {
# 		-1: False,
# 		-2: False,
# 		-3: True,
# 		-4: False,
# 		-5: True,
# 		0: True,
# 		10: False, 
# 		20: False,
# 		40: False,
# 		60: True,
# 		80: False,
# 		100: True,
# 		180: True,
# 		400: False,
# 	},
# 	4: {
# 		0: False,
# 		100: False, 
# 		200: False,
# 		400: False
# 	},
# 	20: {
# 		0: True,
# 		10: False, 
# 		20: False,
# 		40: True,
# 		60: True,
# 		80: False,
# 		100: False,
# 		120: True,
# 		180: False,
# 		400: False,
# 	}
# }
# reducePacketTimeThresholdShow = -1
show = {
	1: {
		0: True
 	},
	2: {
		-1: True,
		-2: True,
		-3: True,
		-4: True,
		-5: True,
		-6: False,
		0: True,
		10: False, 
		20: False,
		40: False,
		60: False,
		80: False,
		100: False,
		180: False,
		400: False,
	},
	4: {
		0: False,
		100: False, 
		200: False,
		400: False
	},
	20: {
		0: True,
		10: False, 
		20: False,
		40: True,
		60: True,
		80: False,
		100: False,
		120: True,
		180: False,
		400: False,
	}
}
pcShowInit = 0

reducePacketTimeThresholdIndex = 0
for reducePacketTimeThreshold in reducePacketTimeThresholds:
	if reducePacketTimeThreshold != -4:
		pcShow = 0
	else:
		pcShow = pcShowInit
		
	for lostPacketThreshold in lostPacketThresholds:
		if lostPacketThreshold not in show:
			continue

		lineIndex = 0
		timeThresholdIndex = -1
		for timeThreshold in timeThresholds:
			timeThresholdIndex += 1
			if reducePacketTimeThreshold == -1 and timeThresholdIndex > 2:
				continue
			if reducePacketTimeThreshold == -2 and (timeThresholdIndex < 3 or timeThresholdIndex > 5):
				continue
			if reducePacketTimeThreshold == -4 and timeThresholdIndex < 6:
				continue
			#if timeThreshold not in show[lostPacketThreshold] or show[lostPacketThreshold][timeThreshold] == False:
			#	continue

			label = r'$t='+timeThreshold.replace('$delay', 'd').replace('25', 'a').replace('*', ' \cdot ').replace('(', '').replace(')', '')
			if reducePacketTimeThreshold == -4:
				if lineIndex == 0:
					label += r'\vee c = 1'
				else:
					label += '\wedge c > 1'
			label += '$'

			delayIndex = 0
			for delay in delays:
				if delay == 10:
					print('r='+str(-reducePacketTimeThreshold)+'E, n='+str(lostPacketThreshold)+', t='+str(timeThreshold)+': '+str(measurements[lostPacketThreshold][timeThreshold][pcShow][reducePacketTimeThreshold][0][delayIndex]))
					break
				delayIndex += 1
			ax[reducePacketTimeThresholdIndex].errorbar(delays, measurements[lostPacketThreshold][timeThreshold][pcShow][reducePacketTimeThreshold][0], measurements[lostPacketThreshold][timeThreshold][pcShow][reducePacketTimeThreshold][1], errorevery=1, linestyle=linestyles[lineIndex], label=label)
			lineIndex += 1
			
	ax[reducePacketTimeThresholdIndex].set_xlim([0,53])
	#ax[reducePacketTimeThresholdIndex].set_xticks([0, 20, 40])
	ax[reducePacketTimeThresholdIndex].set_xticks([0, 10, 20, 30, 40, 50])
	#ax[reducePacketTimeThresholdIndex].set_xticks([10, 30, 50], minor=True)
	ax[reducePacketTimeThresholdIndex].set_ylim([0,3.2])
	ax[reducePacketTimeThresholdIndex].set_yticks([0, 1, 2, 3])
	ax[reducePacketTimeThresholdIndex].set_yticks([0.5, 1.5, 2.5], minor=True)
	ax[reducePacketTimeThresholdIndex].grid(which='both')
	ax[reducePacketTimeThresholdIndex].legend(loc='upper left')
	if reducePacketTimeThreshold == -1:
		ax[reducePacketTimeThresholdIndex].set_title('$r = E$', size='large')
	else:
		ax[reducePacketTimeThresholdIndex].set_title('$r = '+str(-reducePacketTimeThreshold)+' \cdot E$', size='large')
	if reducePacketTimeThresholdIndex == 0:
		ax[reducePacketTimeThresholdIndex].set_ylabel('PTB Detection Time [s]')	
	if reducePacketTimeThresholdIndex == 1:
		ax[reducePacketTimeThresholdIndex].set_xlabel('Bottleneck Delay $\it{d}$ [ms]')
			
	reducePacketTimeThresholdIndex += 1


#ax[0].set_ylabel('Detection Time [s]')

plot_dir = dirname+"plots/decrease/cc-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_cc-limited.pdf")
plt.show()