from scipy.stats import sem, t
from statistics import fmean
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

t0 =  3000
t1 = 103000

senders = 2

def computeTp(run):
	file = path + str(run) + ".vec"
	#print(file)
	conn = sqlite3.connect(file)
	c = conn.cursor()
	
	tps = []
	for sender in range(senders):
		#tps.append([])
		#print("sender " + str(sender+1)) 
	
		sampleNum = 0
		sampleData = 0
		for row in c.execute("""
select sum(value) 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'shared_link.receiver""" + str(sender+1) + """.quic' and vectorName = 'packetReceived:vector(packetBytes)'
) and simtimeRaw between """ + str(t0*1000000000) + " and " + str(t1*1000000000) + """
order by simtimeRaw
"""):
			#sampleNumOfData = int(row[0] / (dt*1000000000))
			#if sampleNum < sampleNumOfData:
			#	# calc
			#	tps[sender].append(sampleData*8/(dt*1000))
			#	sampleNum += 1
			#	for s in range(sampleNum, sampleNumOfData):
			#		tps[sender].append(0)
			#	sampleNum = sampleNumOfData
			#	sampleData = 0
			#print(row[0])
			sampleData += int(row[0])
		tps.append(sampleData*8 / ((t1-t0)*1000))
			
	return tps

# for t1New in range(4000, 103001, 1000):
# for t1New in range(4000, 53001, 1000):
# 	t1 = t1New
# 	tps = computeTp(18)
# 	max = tps[0]
# 	if tps[1] > max: max = tps[1]
# 	mean = (tps[0] + tps[1]) / 2
# 	print(str(t1) + " - 1: " + "{:.3f}".format(tps[0]) + "Mb/s, 2: " + "{:.3f}".format(tps[1]) + "Mb/s, mean: " + "{:.3f}".format(mean) + "Mb/s (" + "{:.2f}".format(((max/mean)-1)*100) +"%)")

runs = 4

print("runs="+str(runs))
for t1New in [13000]: #range(4000, 13001, 1000):
	t1 = t1New
	#print("time=["+str(t0)+","+str(t1)+"]")
	diffs = []
	min = 1
	max = 0
	for run in range(runs):
		tps = computeTp(run)
		print(str(tps[0])+", "+str(tps[1]))
		largerTp = tps[0]
		if tps[1] > largerTp: largerTp = tps[1]
		meanTp = (tps[0] + tps[1]) / 2
		relativeDiffToMean = largerTp/meanTp - 1
		#print("{:.2f}".format(relativeDiffToMean*100) +"%")
		if relativeDiffToMean < min:
			min = relativeDiffToMean
		if relativeDiffToMean > max:
			max = relativeDiffToMean
		diffs.append(relativeDiffToMean)
	
		
	diffs_mean = fmean(diffs)
	
	print("time=["+str(t0)+","+str(t1)+"], mean=" + "{:.2f}".format(diffs_mean*100) + "%, interval=[" + "{:.2f}".format(min*100) + "," + "{:.2f}".format(max*100)+"]%")
	
	# calculate confidence interval
	#confidence = 0.99
	#diffs_std_err = sem(diffs)
	#h = diffs_std_err * t.ppf((1 + confidence) / 2., runs - 1)
	
	#print("time=["+str(t0)+","+str(t1)+"], mean=" + "{:.2f}".format(diffs_mean*100) + "%, ci: " + "{:.2f}".format(h*100) + "%" + ", interval=[" + "{:.2f}".format(min*100) + "," + "{:.2f}".format(max*100)+"]%")
	#print("ci: " + "{:.2f}".format(h*100) + "%")

# runs = 1
#
# tps = []
# for run in range(runs):
# 	tps.append(computeTp(run))
#
# print("calc avg")
# measurements = []
# confidence = 0.95
# for sender in range(senders):
# 	measurements.append([[], []])
# 	for sampleNum in range(int((t1-t0)/dt) + 1):
# 		tps_per_run = []
# 		for run in range(runs):
# 			if sampleNum < len(tps[run][sender]):
# 				tps_per_run.append(tps[run][sender][sampleNum])
#
# 		if len(tps_per_run) == 0:
# 			measurements[sender][0].append(0)
# 			measurements[sender][1].append(0)
# 		else:
# 			tp_mean = fmean(tps_per_run)
# 			measurements[sender][0].append(tp_mean)
#
# 			# calculate confidence interval
# 			std_err = sem(tps_per_run)
# 			h = std_err * t.ppf((1 + confidence) / 2., runs - 1)
# 			measurements[sender][1].append(h)
#
# print("    Time range    | Tp S1 | Tp S2 | Tp S3")
# for sampleNum in range(int((t1-t0)/dt) + 1):
# 	print('{:5d}'.format(sampleNum*dt) + "ms - " + '{:5d}'.format((sampleNum+1)*dt) + "ms | " + "{:.3f}".format(measurements[0][0][sampleNum]) + " | " + "{:.3f}".format(measurements[1][0][sampleNum]) + " | " + "{:.3f}".format(measurements[2][0][sampleNum]))

