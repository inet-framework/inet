import numpy as np
import os
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/blackhole/outage/cc-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
packetLossRates = result['packetLossRates']
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
pcs = result['pcs']
n_ts = result['n_ts']
measurements = result['measurements']

fig, ax = plt.subplots(len(reducePacketTimeThresholds), 1, sharex='col', sharey='row', figsize=(12, 8), dpi=100, tight_layout=True)

packetLossRatesShow = 0.5

rIndex = 0
for r in reducePacketTimeThresholds:
	diffs = []
	posError = []
	negError = []
	for n_t in n_ts:
		diffs.append(0)
		posError.append(measurements[packetLossRatesShow]['false'][1])
		negError.append(-measurements[packetLossRatesShow]['false'][1])
	ax[rIndex].fill_between(n_ts, posError, negError, color="lightgray", label="without")

	for pc in pcs:
		#if pc == 1:# and lostPacketThreshold < 1000000:
			#continue
			
		diffs = []
		#differrs = []
		posError = []
		negError = []
		for n_tIndex in range(len(n_ts)):
			diffs.append(measurements[packetLossRatesShow]['true'][pc][r][0][n_tIndex] - measurements[packetLossRatesShow]['false'][0])
			posError.append(diffs[n_tIndex]+measurements[packetLossRatesShow]['true'][pc][r][1][n_tIndex])
			negError.append(diffs[n_tIndex]-measurements[packetLossRatesShow]['true'][pc][r][1][n_tIndex])
		ax[rIndex].errorbar(n_ts, diffs, measurements[packetLossRatesShow]['true'][pc][r][1], errorevery=1, capsize=2.0, label='pc='+str(pc))
		
	ax[rIndex].legend()
	rIndex += 1

ax[len(reducePacketTimeThresholds)-1].set_xlabel('n_t')

#handles, labels = ax[0][0].get_legend_handles_labels()
#fig.legend(handles, labels, loc=[0.18,0.89], ncol=4)

#fig.supxlabel('Send Rate')
#fig.supylabel('Additional Sent Packets')

plot_dir = dirname + "plots/outage/cc-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"outage_cc-limited.pdf")
plt.show()
