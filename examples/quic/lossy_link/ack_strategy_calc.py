import numpy as np
from scipy.stats import sem, t
from scipy import mean
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

#rtts_measured = [2, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100]
rtts_measured = [2, 4, 6, 8, 10]
#ps = [".001", ".0002", ".00005"]
ps = [".001"]
#pktBeforeAcks = [1, 2, 10]
pktBeforeAcks = [2, 102, 1, 110, 10]
runs = 100
for p_str in ps:
	tps_mean = []
	confidence = 0.95
	tps_err = []
	
	for pktBeforeAcks_index in range(len(pktBeforeAcks)):
		pktBeforeAck = pktBeforeAcks[pktBeforeAcks_index]
		tps_mean.append([])
		tps_err.append([])
		tps_err[pktBeforeAcks_index].append([]) # lower error
		tps_err[pktBeforeAcks_index].append([]) # upper error
		
		for rtts_measured_index in range(len(rtts_measured)):
			rtt = rtts_measured[rtts_measured_index]
			
			tps_measured = []
			for run in range(runs):
				if pktBeforeAck < 100:
					file = path + "ack_strategy-delay="+str(int(rtt/2))+",p="+p_str+",pktBeforeAck="+str(pktBeforeAck)+"-"+str(run)+".vec"
				else:
					file = path + "ack_strategy-delay="+str(int(rtt/2))+",p="+p_str+",pktBeforeAck="+str(pktBeforeAck-100)+",useIBit=true-"+str(run)+".vec"					
				print(file)
				conn = sqlite3.connect(file)
				c = conn.cursor()

				first = 0
				last = 0
				data = 0

				t0 = str( (2 + int(rtt/2)) * 1000000000000 )
				t1 = str( (2 + rtt * 8) * 1000000000000 )
				if p_str == ".00005":
					t1 = str( (2 + rtt * 4) * 1000000000000 )
				#print("measure tp from " + t0 + " to " + t1)
				for row in c.execute("""
select simtimeRaw, value 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'lossy_link.receiver.quic' and vectorName = 'packetReceived:vector(packetBytes)'
) and simtimeRaw between """ + t0 + " and " + t1 + """
order by simtimeRaw
"""):
					if first == 0:
						first = row[0]
					else:
						data += int(row[1])
					last = row[0]

				tp = data*8000000 / (last - first)
				#print(tp)
				tps_measured.append(tp)

			tp_mean = mean(tps_measured)
			tps_mean[pktBeforeAcks_index].append(tp_mean)
			
			# calculate confidence interval
			std_err = sem(tps_measured)
			h = std_err * t.ppf((1 + confidence) / 2., runs - 1)
			tps_err[pktBeforeAcks_index][0].append(h)
			tps_err[pktBeforeAcks_index][1].append(h)
			
	np.save(path + "ack_strategy-p="+p_str, tps_mean)
	np.save(path + "ack_strategy_err-p="+p_str, tps_err)
	
