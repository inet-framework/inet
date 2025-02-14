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
	
	for reducePacketTimeThreshold in reducePacketTimeThresholds:
		#print("plot for pktPerSecs: "+str(pktPerSecs))
		for lostPacketThreshold in lostPacketThresholds:

			linestylesIndex = 0
			for timeThreshold in timeThresholds:
				bases = measurements[delay][msgLength]['true'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0]
				diffs = []
				errs = []
				for i in range(len(bases)):
					diff = measurements[delay][msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0][i] - bases[i]
					diffs.append(diff * 1000)
					err = measurements[delay][msgLength]['false'][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][1][i]
					errs.append(err * 1000)
				msgLengthStr = msgLength + ' B'
				if msgLength == 'intuniform(1042,1442)':
					msgLengthStr = '[1042, 1442] B'
				label = msgLengthStr
				#ax.errorbar(pktPerSecs, diffs, errs, errorevery=1, label=label)
				labelBase = label
				for ptb in ['false', 'true']:
					#label = labelBase+', ptb='+ptb
					linestyle = '-'
					if ptb == 'true':
						if msgLength != 'intuniform(1042,1442)':
							continue
						label = 'w/ PTB msg'
						linestyle = '--'
					vals = measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][0]
					errs = measurements[delay][msgLength][ptb][lostPacketThreshold][timeThreshold][pc][reducePacketTimeThreshold]['ptbDetectionTimes'][1]
					for i in range(len(vals)):
						vals[i] = vals[i] * 1000
						errs[i] = errs[i] * 1000
					ax.errorbar(pktPerSecs, vals, errs, errorevery=1, linestyle=linestyle, label=label)

ax.set_xscale('log')
ax.set_xlim([0.9,20000])
ax.set_xticks([1, 10, 100, 1000, 10000])
ax.set_xticklabels(['1', '10', '100', '1000', '10000'])
ax.set_yticks([200, 400, 600, 800], minor=False)
ax.set_yticks([100, 300, 500, 700, 900], minor=True)
ax.set_ylim([0, 980])
ax.grid()
ax.legend(loc='upper right', ncol=2)
ax.set_xlabel('Send Rate [msg/s]')
ax.set_ylabel('PTB Detection Time [ms]')
#ax.set_title('delay = '+str(delay)+' ms')

plot_dir = dirname+"plots/decrease/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"decrease_time_d"+str(delay)+".pdf")
plt.show()