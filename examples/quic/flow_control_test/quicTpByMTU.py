#!/usr/bin/env python3
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
	) and simtimeRaw between 2000000000000 and 100000000000000
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
print("rwndStreamLimit = 65KB")
print("BtlLinkDelay = 10")
print(getQuicTp(dirname + "results/QuicTpByMTU=10.vec"))
print(getQuicTp(dirname + "results/General-rwndLimit=4294967295,rwndStreamLimit=66560,delay=10,mtu=1280-#0.vec"))
print("---------------------------------------------")
print("rwndStreamLimit = 1000000")
print("BtlLinkDelay = 10")
print(getQuicTp(dirname + "results/General-rwndLimit=4294967295,rwndStreamLimit=1000000,delay=10,mtu=1280-#0.vec"))