import numpy as np
from scipy.stats import sem, t
from scipy import mean
import sqlite3
import os
import sys

algs = [ "Up", "Down", "OptUp", "Binary", "Jump" ]
mtus = np.arange(1280, 1501, 4)

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

times = []
for alg_index in range(len(algs)):
	alg = algs[alg_index]
	times.append([])
	
	for mtu_index in range(len(mtus)):
		mtu = mtus[mtu_index]
		times[alg_index].append([])
		
		#General-alg=Binary,mtu=1280-#0.sca
		file = path + 'transmission-alg="'+alg+'",pmtu='+str(mtu)
		#print(file)
		conn = sqlite3.connect(file + ".sca")
		c = conn.cursor()
		simNumOfTests = -1
		simSearchTime = -1.0
		simNetworkLoad = -1
		for row in c.execute("""
	select scalarName, scalarValue 
	from scalar 
	where moduleName = 'Simple2.sender.quic'
	"""):
			if row[0] == "dplpmtudSearchNumberOfTests_cid=0_pid=0:last":
				simNumOfTests = int(row[1])
			elif row[0] == "dplpmtudSearchTime_cid=0_pid=0:last":
				simSearchTime = row[1]
			elif row[0] == "dplpmtudSearchNetworkLoad_cid=0_pid=0:last":
				simNetworkLoad = int(row[1])
		c.close()

		conn = sqlite3.connect(file + ".vec")
		c = conn.cursor()
		
		lastPmtu = 0
		durations = 0
		simAvgPmtu = 0
		for row in c.execute("""
	select simtimeRaw, value 
	from vectorData 
	where vectorId = (
		select vectorId 
		from vector 
		where moduleName = 'Simple2.sender.quic' and vectorName = 'dplpmtudSearchPmtu_cid=0_pid=0:vector'
	)
	order by simtimeRaw
	"""):
			time = row[0]/1000000000 # in ms
			pmtu = row[1]
			if lastPmtu != 0:
				duration = time - lastTime
				#print("had " + str(lastPmtu) + " for " + str(duration))
				simAvgPmtu += lastPmtu * duration
				durations += duration
			else:
				time += 20 # search algorithm starts 20ms later than the first value were recorded
			
			lastTime = time
			lastPmtu = pmtu
		c.close()
	
		if (durations > 0):
			simAvgPmtu /= durations
		else:
			simAvgPmtu = lastPmtu
		#print(str(mtu) + " " + str(simAvgPmtu))

		times[alg_index][mtu_index] = simSearchTime
		#metrics[alg_index][mtu_index].append(simNumOfTests)
		#metrics[alg_index][mtu_index].append(simSearchTime)
		#metrics[alg_index][mtu_index].append(simNetworkLoad)
		#metrics[alg_index][mtu_index].append(simAvgPmtu)
		
np.save(path + "transmission_time", times)
	
