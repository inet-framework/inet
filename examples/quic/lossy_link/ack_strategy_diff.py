import numpy as np
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

rtts_measured = np.arange(2, 11, step=2)
ps = [".001", ".0002", ".00005"]
pktBeforeAcks = [1, 2, 10]
pktBeforeAck1_index = 0
pktBeforeAck2_index = 1
pktBeforeAck10_index = 2
for p_str in ps:
	print("p = " + p_str)
	tps_measured = np.load(path+"ack_strategy-p="+p_str+".npy")
	#print(tps_measured)
	
	for rtts_index in range(len(rtts_measured)):
		rtt = rtts_measured[rtts_index]
		print("rtt = " + str(rtt))
		
		tp_1 = tps_measured[pktBeforeAck1_index][rtts_index]
		tp_2 = tps_measured[pktBeforeAck2_index][rtts_index]
		tp_10 = tps_measured[pktBeforeAck10_index][rtts_index]
		print("tp_1 = " + str(tp_1))
		print("tp_2 = " + str(tp_2) + " " + str(tp_2-tp_1) + " " + str((tp_2-tp_1)/tp_1 * 100) + "%")
		print("tp_10 = " + str(tp_10) + " " + str(tp_10-tp_1) + " " + str((tp_10-tp_1)/tp_1 * 100) + "%")
		
		#print("diff: " + str( (tp_1-tp_10)/tp_1 * 100 ))