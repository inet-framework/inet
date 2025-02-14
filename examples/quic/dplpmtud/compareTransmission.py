import numpy as np
from scipy.stats import sem, t
from scipy import mean
import sqlite3
import os
import sys
sys.path.append('/Users/msvoelker/Documents/Forschung/Diss/Paper/2020-09 DPLPMTUD Methods/scripts')
import pmtudAlgs

algs = [ "Up", "Down", "OptUp", "Binary", "Jump" ]
if len(sys.argv) > 1:
	algs = [ sys.argv[1] ]
	
mtus = np.arange(pmtudAlgs.minMtu, pmtudAlgs.maxMtu+1, pmtudAlgs.stepSize)
if len(sys.argv) > 2:
	mtus = [ int(sys.argv[2]) ]

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

for alg in algs:
	for mtu in mtus:
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

		# without PTB
		pmtudAlgs.pmtud(alg, mtu, False)
		if simNumOfTests != pmtudAlgs.pmtudFunctions.getNumOfTests():
			print(str(mtu) + ": number of tests different " + str(simNumOfTests) + " != " + str(pmtudAlgs.pmtudFunctions.getNumOfTests()))
		
		if round(simSearchTime, 4) != round(pmtudAlgs.pmtudFunctions.getTime(), 4):
			print(str(mtu) + ": time different " + str(simSearchTime) + " != " + str(pmtudAlgs.pmtudFunctions.getTime()))
		
		if simNetworkLoad != pmtudAlgs.pmtudFunctions.getNetload():
			print(str(mtu) + ": network load different " + str(simNetworkLoad) + " != " + str(pmtudAlgs.pmtudFunctions.getNetload()))
		
		if round(simAvgPmtu, 1) != round(pmtudAlgs.pmtudFunctions.getAvgPmtu(), 1):
			print(str(mtu) + ": avg PMTU different " + str(round(simAvgPmtu, 1)) + " != " + str(round(pmtudAlgs.pmtudFunctions.getAvgPmtu(), 1)))
		
	#	pmtudAlgs.pmtud(alg, mtu, True)
	#	result[alg]["Ptb"].append(pmtudAlgs.pmtudFunctions.getNumOfTests())

		#np.save(path + "ack_strategy-p="+p_str, tps_mean)
	
		#np.save(path + "ack_strategy_err-p="+p_str, tps_err)
	
