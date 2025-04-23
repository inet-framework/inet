from statistics import fmean
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

t0 =  3000
t1 = 13000

senders = 2
runs = 4

def computeTp(run):
	file = path + str(run) + ".vec"
	#print(file)
	conn = sqlite3.connect(file)
	c = conn.cursor()
	
	tps = []
	for sender in range(senders):
		c.execute("""
select sum(value) 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'shared_link.receiver""" + str(sender+1) + """.quic' and vectorName = 'packetReceived:vector(packetBytes)'
) and simtimeRaw between """ + str(t0*1000000000) + " and " + str(t1*1000000000) + """
order by simtimeRaw
""")
		receivedData = int(c.fetchone()[0])
		tps.append(receivedData*8 / ((t1-t0)*1000))
			
	return tps


diffs = []
for run in range(runs):
	tps = computeTp(run)
	largerTp = tps[0]
	if tps[1] > largerTp: largerTp = tps[1]
	meanTp = (tps[0] + tps[1]) / 2
	relativeDiffToMean = largerTp/meanTp - 1
	diffs.append(relativeDiffToMean)

diffs_mean = fmean(diffs)

print("mean=" + "{:.2f}".format(diffs_mean*100) + "%")
print("mean <= 5%: " + ("pass" if diffs_mean <= .05 else "fail"))
