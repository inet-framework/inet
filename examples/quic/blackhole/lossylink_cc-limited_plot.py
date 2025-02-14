import numpy as np
import os
import matplotlib.pyplot as plt

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/blackhole/lossylink/cc-limited/"

result = np.load(path+"result.npy", allow_pickle=True).item()
packetLossRates = result['packetLossRates']
lostPacketThresholds = result['lostPacketThresholds']
timeThresholds = result['timeThresholds']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
#pcs = result['pcs']
n_ts = result['n_ts']
measurements = result['measurements']

fig, ax = plt.subplots(len(reducePacketTimeThresholds), 1, sharex='col', sharey='row', figsize=(12, 8), dpi=100, tight_layout=True)

packetLossRatesShow = 0.02

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

	diffs = []
	for n_tIndex in range(len(n_ts)):
		diffs.append(measurements[packetLossRatesShow]['true'][r][0][n_tIndex] - measurements[packetLossRatesShow]['false'][0])
	ax[rIndex].errorbar(n_ts, diffs, measurements[packetLossRatesShow]['true'][r][1], errorevery=1, capsize=2.0)
		
	ax[rIndex].legend()
	rIndex += 1

ax[len(reducePacketTimeThresholds)-1].set_xlabel('n_t')

plot_dir = dirname + "plots/lossylink/cc-limited/"
if not os.path.isdir(plot_dir):
    os.makedirs(plot_dir)
fig.savefig(plot_dir+"lossylink_cc-limited.pdf")
plt.show()
