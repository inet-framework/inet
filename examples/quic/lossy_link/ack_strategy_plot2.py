import matplotlib.pyplot as plt
import numpy as np
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

width = 0.25
annotateSep = -.05
rtts_measured = np.arange(2, 11, step=2)
x = np.arange(len(rtts_measured))
pktBeforeAcks = [2, 102, 1, 110, 10]
tps_mean = np.load(path+"ack_strategy-p=.001.npy")
tps_err = np.load(path+"ack_strategy_err-p=.001.npy")

tps_mean_norm = []
offset = 50
for index in range(len(tps_mean)):
	tps_mean_norm.append(tps_mean[index]-tps_mean[2]+offset)

labels = ["ack every other packet", "ack every other packet with I-Bit", "ack every packet", "ack every 10th packet with I-Bit", "ack every 10th packet"]
colors = ["#2874a6", "#aed6f1", "#239b56", "#f5b7b1", "#b03a2e"]
plt.figure(figsize=(7.6, 4.8), dpi=200, tight_layout=True)
for index in range(len(tps_mean)):
	plt.bar(rtts_measured + width*(index-2), tps_mean_norm[index], width, label=labels[index], yerr=tps_err[index], capsize=2.0, color=colors[index])
	if index == 0 or index == len(tps_mean)-1:
		for rtt_index in range(len(rtts_measured)):
			rtt = rtts_measured[rtt_index]
			tp_1 = tps_mean[2][rtt_index]
			tp_n = tps_mean[index][rtt_index]
			if index == 0:
				#plt.annotate(str(round((tp_n-tp_1)/tp_1 * 100, 1)) + "%", (rtt + 1.5*width*(index-2) - annotateSep, tps_mean[index][rtt_index]), rotation=90, verticalalignment='center', horizontalalignment='right')
				plt.annotate(str(round((tp_n-tp_1)/tp_1 * 100, 1)) + "%", (rtt + 1.5*width*(index-2) - annotateSep, tps_mean_norm[index][rtt_index]), rotation=90, verticalalignment='center', horizontalalignment='right')
			if index == len(tps_mean)-1:
				plt.annotate(str(round((tp_n-tp_1)/tp_1 * 100, 1)) + "%", (rtt + 1.5*width*(index-2) + annotateSep, tps_mean_norm[index][rtt_index]), rotation=90, verticalalignment='center')
	if index == 2: # add text on the bars for pktBeforeAck = 1
		for rtt_index in range(len(rtts_measured)):
			rtt = rtts_measured[rtt_index]
			plt.text(rtt+.02, 45, "TP1="+str(round(tps_mean[2][rtt_index], 2)), rotation=90, color='w', horizontalalignment='center', verticalalignment='top')
			#plt.text(rtt+.02, 55, "TP1="+str(round(tps_mean[2][rtt_index], 2)), horizontalalignment='center')
		
#locs, labels = plt.xticks()
#print(locs)
#print(labels)
#locs.append(2)
#plt.xticks(locs, labels)
#plt.xticks(list(plt.xticks()[0]) + [10, 40, 80, 120, 160, 190])
#plt.xticks([10, 40, 80, 120, 160, 190], ["s1 starts", "s2 starts", "s3 starts", "s1 stops", "s2 stops", "s3 stops"], rotation=20)
plt.yticks([0, 10, 20, 30, 40, 50], ["-50", "-40", "-30", "-20", "-10", "TP1"]) #, rotation=80)
#plt.yticks([0, 10, 20, 30, 40, 50, 60], ["-50", "-40", "-30", "-20", "-10", "TP1", ""]) #, rotation=80)

plt.xlabel('RTT (ms)')
plt.ylabel('Throughput (Mbit/s)')
plt.legend(loc='lower right')
#plt.show()
plt.savefig(path+"ack_strategy.png")
