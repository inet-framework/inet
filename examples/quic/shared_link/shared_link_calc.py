import numpy as np
from scipy.stats import sem, t
from scipy import mean
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/shared_link/"

t0 = 0
t1 = 200000
dt = 1000

senders = 3

def computeTp(run):
	file = path + str(run) + ".vec"
	print(file)
	conn = sqlite3.connect(file)
	c = conn.cursor()
	
	tps = []
	for sender in range(senders):
		tps.append([])
#		tps[sender].append(0)
		print("sender " + str(sender+1)) 
#		for t in range(t0, t1, dt):
#			c.execute("""
#select ifnull(sum(value), 0)
#from vectorData 
#where vectorId = (
#	select vectorId 
#	from vector 
#	where moduleName = 'shared_link.receiver""" + str(sender+1) + """.quic' and vectorName = 'packetReceived:vector(packetBytes)'
#) and simtimeRaw between """ + str(t*1000000000) + " and " + str((t+dt)*1000000000) + """
#order by simtimeRaw
#""")
#			row = c.fetchone()
#			#print(str(t) + ": " + str(row[0]*8/1000000))
#			tps[sender].append(row[0]*8/(dt*1000))
	
		sampleNum = 0
		sampleData = 0
		for row in c.execute("""
select simtimeRaw, value 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'shared_link.receiver""" + str(sender+1) + """.quic' and vectorName = 'packetReceived:vector(packetBytes)'
) and simtimeRaw between """ + str(t0*1000000000) + " and " + str(t1*1000000000) + """
order by simtimeRaw
"""):
			sampleNumOfData = int(row[0] / (dt*1000000000))
			if sampleNum < sampleNumOfData:
				# calc
				#print(str(sampleNum) + ": " + str(sampleData*8/(dt*1000)))
				tps[sender].append(sampleData*8/(dt*1000))
				sampleNum += 1
				for s in range(sampleNum, sampleNumOfData):
					tps[sender].append(0)
				sampleNum = sampleNumOfData
				sampleData = 0
				
			sampleData += int(row[1])
			
	return tps

runs = 100

tps = []
for run in range(runs):
	tps.append(computeTp(run))

print("calc avg")
# tps_avg = []
# tps_err = []
# confidence = 0.95
# for sender in range(senders):
# 	tps_avg.append([])
# 	tps_err.append([])
# 	for sampleNum in range(int((t1-t0)/dt) + 1):
# 		tps_per_run = []
# 		for run in range(runs):
# 			if sampleNum < len(tps[run][sender]):
# 				tps_per_run.append(tps[run][sender][sampleNum])
#
# 		if len(tps_per_run) == 0:
# 			tps_avg[sender].append(0)
# 			tps_err[sender].append(0)
# 		else:
# 			tp_mean = mean(tps_per_run)
# 			tps_avg[sender].append(tp_mean)
#
# 			# calculate confidence interval
# 			std_err = sem(tps_per_run)
# 			h = std_err * t.ppf((1 + confidence) / 2., runs - 1)
# 			tps_err[sender].append(h)
measurements = []
confidence = 0.95
for sender in range(senders):
	measurements.append([[], []])
	for sampleNum in range(int((t1-t0)/dt) + 1):
		tps_per_run = []
		for run in range(runs):
			if sampleNum < len(tps[run][sender]):
				tps_per_run.append(tps[run][sender][sampleNum])
		
		if len(tps_per_run) == 0:
			measurements[sender][0].append(0)
			measurements[sender][1].append(0)
		else:
			tp_mean = mean(tps_per_run)
			measurements[sender][0].append(tp_mean)
		
			# calculate confidence interval
			std_err = sem(tps_per_run)
			h = std_err * t.ppf((1 + confidence) / 2., runs - 1)
			measurements[sender][1].append(h)

result = {
	"senders": senders,
	"measurements": measurements
}

np.save("shared_link", result, allow_pickle=True)
#np.save(path + "shared_link_tps_avg", tps_avg)
#np.save(path + "shared_link_tps_err", tps_err)

