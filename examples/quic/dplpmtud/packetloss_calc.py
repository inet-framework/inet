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
path = dirname + "../../../results/quic/dplpmtud/packetloss/"

plrs = np.arange(0, 0.11, 0.01)
runs = 1000
algs = ["OptBinary"] #["OptBinary", "Jump"]

measurements = {}

for alg in algs:
	
	measurements[alg] = {
        'times': [[], []],
        'maxProbeNumbers': [[], []]
    }
	
	for plr in plrs:
		plr = round(plr, 2) if plr > 0 else int(plr)
		
		times = []
		maxProbeNumbers = []
		for run in range(runs):
			if plr == 0 and run > 0:
				break
				
			file = path + 'alg="'+alg+'"/lossRate='+str(plr)+'/'+str(run)
            #print(file)
			conn = None
			try:
				conn = sqlite3.connect('file:'+file+'.sca?mode=ro', uri=True)
			except sqlite3.OperationalError as e:
				raise RuntimeError('Cannot open '+file) from e
			c = conn.cursor()
			c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Bottleneck2.sender.quic' and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last'
""")
			time = c.fetchone()[0]
			times.append(time)
			c.close()
			conn.close()
			
			try:
				conn = sqlite3.connect('file:'+file+'.vec?mode=ro', uri=True)
			except sqlite3.OperationalError as e:
				raise RuntimeError('Cannot open '+file) from e
			c = conn.cursor()
			c.execute("""
select max(value)
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Bottleneck2.sender.quic' and vectorName = 'dplpmtudNumOfProbesBeforeAck_cid=0_pid=0:vector'
)
""")
			maxProbeNumber = c.fetchone()[0]
			maxProbeNumbers.append(maxProbeNumber)
			c.close()
			conn.close()
		
		timesMean = 0
		timesCi = 0
		if len(times) == 1:
			timesMean = times[0]
		else:
			timesMean, timesCi = mean_confidence_interval(times)
		measurements[alg]['times'][0].append(timesMean)
		measurements[alg]['times'][1].append(timesCi)
		
		maxProbesMean = 0
		maxProbesCi = 0
		if len(maxProbeNumbers) == 1:
			maxProbesMean = maxProbeNumbers[0]
		else:
			maxProbesMean, maxProbesCi = mean_confidence_interval(maxProbeNumbers)
		measurements[alg]['maxProbeNumbers'][0].append(maxProbesMean)
		measurements[alg]['maxProbeNumbers'][1].append(maxProbesCi)
	
		print(alg+" plr="+str(plr)+": time="+str(timesMean)+", maxProbeNumber="+str(maxProbesMean))

result = {
	'algs': algs,
	'plrs': plrs,
	'measurements': measurements
}

np.save("packetloss", result, allow_pickle=True)
