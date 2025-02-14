import matplotlib.pyplot as plt
import numpy as np
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

width = 0.23
annotateSep = -.05
rtts_measured = np.arange(2, 11, step=2)
x = np.arange(len(rtts_measured))
pktBeforeAcks = [2, 102, 1, 110, 10]
tps_mean = np.load(path+"ack_strategy-p=.001.npy")
tps_err = np.load(path+"ack_strategy_err-p=.001.npy")

labels = ["ack every other packet", "ack every other packet with I-Bit", "ack every packet", "ack every 10th packet with I-Bit", "ack every 10th packet"]
colors = ["#2874a6", "#aed6f1", "#239b56", "#f5b7b1", "#b03a2e"]
for index in range(len(tps_mean)):
	plt.bar(rtts_measured + width*(index-2), tps_mean[index], width, label=labels[index], yerr=tps_err[index], capsize=2.0, color=colors[index])
	if index == 0 or index == len(tps_mean)-1:
		for rtt_index in range(len(rtts_measured)):
			rtt = rtts_measured[rtt_index]
			tp_1 = tps_mean[2][rtt_index]
			tp_n = tps_mean[index][rtt_index]
			if index == 0:
				plt.annotate(str(round((tp_n-tp_1)/tp_1 * 100, 1)) + "%", (rtt + 1.5*width*(index-2) - annotateSep, tps_mean[index][rtt_index]), rotation=90, verticalalignment='center', horizontalalignment='right')
			if index == len(tps_mean)-1:
				plt.annotate(str(round((tp_n-tp_1)/tp_1 * 100, 1)) + "%", (rtt + 1.5*width*(index-2) + annotateSep, tps_mean[index][rtt_index]), rotation=90, verticalalignment='center')
		

plt.xlabel('RTT (ms)')
plt.ylabel('Throughput (Mbit/s)')
plt.legend()
plt.show()

