from scipy.stats import sem, t
from statistics import fmean
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

t0 = 0
t1 = 14000
dt = 100

senders = 3

def computeTp(run):
	file = path + str(run) + ".vec"
	print(file)
	conn = sqlite3.connect(file)
	c = conn.cursor()
	
	tps = []
	for sender in range(senders):
		tps.append([])
		print("sender " + str(sender+1)) 
	
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
				tps[sender].append(sampleData*8/(dt*1000))
				sampleNum += 1
				for s in range(sampleNum, sampleNumOfData):
					tps[sender].append(0)
				sampleNum = sampleNumOfData
				sampleData = 0
				
			sampleData += int(row[1])
			
	return tps

runs = 10

tps = []
for run in range(runs):
	tps.append(computeTp(run))

print("calc avg")
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
			tp_mean = fmean(tps_per_run)
			measurements[sender][0].append(tp_mean)
		
			# calculate confidence interval
			std_err = sem(tps_per_run)
			h = std_err * t.ppf((1 + confidence) / 2., runs - 1)
			measurements[sender][1].append(h)

print("    Time range    | Tp S1 | Tp S2 | Tp S3")
for sampleNum in range(int((t1-t0)/dt) + 1):
	print('{:5d}'.format(sampleNum*dt) + "ms - " + '{:5d}'.format((sampleNum+1)*dt) + "ms | " + "{:.3f}".format(measurements[0][0][sampleNum]) + " | " + "{:.3f}".format(measurements[1][0][sampleNum]) + " | " + "{:.3f}".format(measurements[2][0][sampleNum]))

