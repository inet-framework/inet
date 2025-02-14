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
path = dirname + "../../../results/quic/parallel/"

msgLengths = [0, 1000, 10000, 100000, 1000000, 10000000, 100000000]
pmtus = range(1280, 1501, 4)
modes = [0, 1, 2]
ptbs = ['true', 'false']
measurements = {}

def getAppLastRecv(pmtu, msgLength, mode, ptb):
	#parallel/msgLength=${msgLength}/pmtu=${pmtu}/${repetition}.vec
	file = path + 'mode='+str(mode)+'/ptb='+ptb+'/msgLength='+str(msgLength)+'/pmtu='+str(pmtu)+'/0.vec'
	
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

def getNumberOfSentPackets(pmtu, msgLength, mode, ptb):
	#parallel/msgLength=${msgLength}/pmtu=${pmtu}/${repetition}.vec
	file = path + 'mode='+str(mode)+'/ptb='+ptb+'/msgLength='+str(msgLength)+'/pmtu='+str(pmtu)+'/0.sca'
	
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

def getSearchTime(pmtu, msgLength, mode, ptb):
	file = path + 'mode='+str(mode)+'/ptb='+ptb+'/msgLength='+str(msgLength)+'/pmtu='+str(pmtu)+'/0.sca'
	
	conn = None
	try: 
		conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
	except sqlite3.OperationalError as e:
		raise RuntimeError('Cannot open '+file) from e
	c = conn.cursor()
	c.execute("""
select scalarValue 
from scalar 
where moduleName = 'Parallel.sender.quic' 
  and scalarName = 'dplpmtudSearchTime_cid=0_pid=0:last';
""")
	row = c.fetchone()
	searchTime = row[0]*1000
	
	c.close()
	conn.close()
		
	return searchTime	
	
for mode in modes:
	measurements[mode] = {}
	
	for ptb in ptbs:
		measurements[mode][ptb] = {}
		
		for msgLength in msgLengths:
			measurements[mode][ptb][msgLength] = {
				'recvTime': [],
				'sentPackets': [],
				'searchTime': []
			}
			for pmtu in pmtus:
				if msgLength == 0:
					measurements[mode][ptb][msgLength]['recvTime'].append(0)
				else:
					measurements[mode][ptb][msgLength]['recvTime'].append(getAppLastRecv(pmtu, msgLength, mode, ptb))
				measurements[mode][ptb][msgLength]['sentPackets'].append(getNumberOfSentPackets(pmtu, msgLength, mode, ptb))
				measurements[mode][ptb][msgLength]['searchTime'].append(getSearchTime(pmtu, msgLength, mode, ptb))

result = {
	'modes': modes,
	'ptbs': ptbs,
	'msgLengths': msgLengths,
	'pmtus': pmtus,
	'measurements': measurements
}

np.save("parallel", result, allow_pickle=True)
