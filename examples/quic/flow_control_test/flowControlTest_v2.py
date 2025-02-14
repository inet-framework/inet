import sqlite3
import matplotlib.pyplot as plt
import os
import numpy as np
#--------------------------------------------------------------------
#Theoretical calculation
fcLimits = [66560, 102400, 143360]
rtt = np.arange(0.001, 110.0, 0.1)
for fcLimit in fcLimits:
	gp = int(fcLimit) *8 / (1000 * rtt)	
	plt.plot(rtt, gp)
#--------------------------------------------------------------------
#Simulation results

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

rtts_measured = [2, 4, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
fcLimits_measured = [66560, 102400, 143360]
tps_measured = []

for fcLimit_measured in fcLimits_measured:
	for rtt in rtts_measured:
		file = path + "flowControlByFCLimit_delay=" + str(int(rtt/2)) +"_limit=" + str(fcLimit_measured) +".vec"
		print(file)
		conn = sqlite3.connect(file)
		c = conn.cursor()

		first = 0
		last = 0
		data = 0

		for row in c.execute("""
		select simtimeRaw, value 
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'bottleneck_link.server.quic' and vectorName = 'streamRcvDataBytes_cid=0_sid=0:vector'
		) and simtimeRaw between 2000000000000 and 90000000000000
		order by simtimeRaw
		"""):
			if first == 0:
				first = row[0]
			else:
				data += int(row[1])
			last = row[0]
		
		conn.close()

		tp = (data*8000000 / (last - first))
		print(tp)
		tps_measured.append(tp)
		
		if fcLimit_measured == 66560:
			plt.plot(rtt, tp, 'bo', markersize=3, label="initialMaxData: 65 kB")

		if fcLimit_measured == 102400:
			plt.plot(rtt, tp, 'ro', markersize=3, label="initialMaxData: 100 kB")	
			
		if fcLimit_measured == 143360:
			plt.plot(rtt, tp, 'go', markersize=3, label="initialMaxData: 140 kB")	
		
#--------------------------------------------------------------------
plt.xlabel('RTT [ms]')
plt.ylabel('Goodput [Mbit/s]')
plt.axis([0, 110, 0, 120])
plt.suptitle('QUIC Goodput by Flow Control Limits')
plt.legend()
plt.show()
#--------------------------------------------------------------------
