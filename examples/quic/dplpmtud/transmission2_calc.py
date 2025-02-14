import numpy as np
import scipy.stats
import sqlite3
import os

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return m, h

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/dplpmtud/transmission2/"

runs = 100
algs = ["OptBinary"]
pmtus = {
    "OptBinary": [1284]
}
pktIntervals = [15848.9319, 12589.2541, 10000, 7943.2823, 6309.5734, 5011.8723, 3981.0717, 3162.2777, 2511.8864, 1995.2623, 1584.8932, 1258.9254, 1000, 794.3282, 630.9573, 501.1872, 398.1072, 316.2278, 251.1886, 199.5262, 158.4893, 125.8925, 100, 79.4328, 63.0957, 50.1187, 39.8107, 31.6228, 25.1189, 19.9526, 15.8489, 12.5893, 10, 7.9433, 6.3096, 5.0119, 3.9811, 3.1623, 2.5119, 1.9953, 1.5849, 1.2589, 1, 0.7943, 0.631, 0.5012, 0.3981]

times = {}

for alg in algs:
	times[alg] = {}
	
	for pmtu in pmtus[alg]:
		times[alg][pmtu] = [[], []]	

		for pktInterval in pktIntervals:
			#pktIntervalStr = f'{pktInterval:.2f}' if pktInterval > 0 else "0"
		
			timesPerRun = []
			for run in range(runs):
				if pktInterval == 0 and run > 0: 
					break
			
				file = path + 'alg="'+str(alg)+'"/pmtu='+str(pmtu)+'/pktInterval='+str(pktInterval)+'/pktBeforeAck=2/'+str(run)+'.sca'
				#print(file)
                
				conn = None
				try:
					conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
				except sqlite3.OperationalError as e:
					raise RuntimeError('Cannot open '+file) from e
				c = conn.cursor()
				c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck.sender.quic' and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last'
""")
				time = c.fetchone()[0]
				timesPerRun.append(time)
				c.close()
				
			mean = 0
			ci = 0
			if len(timesPerRun) == 1:
				mean = np.mean(timesPerRun)
			else:
				mean,ci = mean_confidence_interval(timesPerRun)
			
			times[alg][pmtu][0].append(mean)
			times[alg][pmtu][1].append(ci)
			#times[algIndex][pmtuIndex].append(np.mean(timesPerRun))

pktPerSecs = []
for pktInterval in pktIntervals:
	pktPerSec = 1000/pktInterval if pktInterval > 0 else 0
	pktPerSecs.append(pktPerSec)

result = {
	"algs": algs,
	"pmtus": pmtus,
	"pktPerSecs": pktPerSecs,
	"times": times
}

np.save("transmission2", result, allow_pickle=True)
	
