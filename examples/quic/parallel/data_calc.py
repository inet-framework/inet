import numpy as np
import scipy.stats
import sqlite3
import os
import matplotlib.pyplot as plt

def mean_confidence_interval(data, confidence=0.95):
	a = 1.0 * np.array(data)
	n = len(a)
	m, se = np.mean(a), scipy.stats.sem(a)
	h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
	return m, h

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/parallel/data/"

msgLengths = [1000, 1585, 2512, 3981, 6310, 10000, 15849, 25119, 39811, 63096, 100000, 158489, 251189, 398107, 630957, 1000000, 1584893, 2511886, 3981072, 6309573, 10000000, 15848932, 25118864, 39810717, 63095734, 100000000]
pmtus = range(1280, 1501, 4)
modes = ['parallel', 'single']
measurements = {}

def getAppLastRecv(pmtu, msgLength, mode):
	#parallel/msgLength=${msgLength}/pmtu=${pmtu}/${repetition}.vec
	file = path + mode + '/msgLength='+str(msgLength)+'/pmtu='+str(pmtu)+'/0.vec'
	
	conn = None
	try: 
		conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
	except sqlite3.OperationalError as e:
		raise RuntimeError('Cannot open '+file) from e
	
	c = conn.cursor()
	c.execute("""
select simtimeRaw
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'Parallel.receiver.app[0]' 
	  and vectorName = 'bytesReceived:vector'
)
order by simtimeRaw desc
limit 1
""")
	row = c.fetchone()
	lastReceive = row[0] / 1000000000
	c.close()

	conn.close()
	return lastReceive

def getNumberOfSentPackets(pmtu, msgLength, mode):
	#parallel/msgLength=${msgLength}/pmtu=${pmtu}/${repetition}.vec
	file = path + mode + '/msgLength='+str(msgLength)+'/pmtu='+str(pmtu)+'/0.sca'
	
	conn = None
	try: 
		conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
	except sqlite3.OperationalError as e:
		raise RuntimeError('Cannot open '+file) from e
	
	c = conn.cursor()
	c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Parallel.sender.ppp[0].ppp' 
and scalarName = 'txPk:count';
""")
	row = c.fetchone()
	sentPacket = row[0]
	c.close()
	conn.close()
	
	return sentPacket
	
	
for mode in modes:
	measurements[mode] = {}
	for msgLength in msgLengths:
		measurements[mode][msgLength] = {
			'recvTime': [],
			'sentPackets': []
		}
		for pmtu in pmtus:
			measurements[mode][msgLength]['recvTime'].append(getAppLastRecv(pmtu, msgLength, mode))
			measurements[mode][msgLength]['sentPackets'].append(getNumberOfSentPackets(pmtu, msgLength, mode))

result = {
	'modes': modes,
	'msgLengths': msgLengths,
	'pmtus': pmtus,
	'measurements': measurements
}

np.save("data", result, allow_pickle=True)
