#!/usr/bin/env python3.9
import numpy as np
import sqlite3
import os

def getQuicTp(filename):
	conn = sqlite3.connect(filename)
	c = conn.cursor()

	first = 0
	last = 0
	data = 0

	for row in c.execute("""
	select simtimeRaw, value 
	from vectorData 
	where vectorId = (
		select vectorId 
		from vector 
		where moduleName = 'bottleneck_link.server.quic' and vectorName = 'packetReceived:vector(packetBytes)'
	) and simtimeRaw between 5000000000000 and 10000000000000
	order by simtimeRaw
	"""):
		if first == 0:
			first = row[0]
		else:
			data += int(row[1])
		last = row[0]

	conn.close()
	return data*8000000 / (last - first)


dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

mtus = ['1280', '1452', '1500', '9000']
tp = []
for mtu in mtus:
	tp.append(getQuicTp(path + "QuicTpByMTU=" + mtu + ".vec"))

result = {
	'mtus': mtus,
	'tp': tp
}
np.save("quicTpByMTU", result, allow_pickle=True)
