import sqlite3
import os
import numpy as np

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/flow_control_limited/validation/"

#rtts = [2, 4, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
rtts = np.arange(4, 101, 4)
initialMaxDataSizes = [65, 100, 140]
gps_measured = {}
start = 3000000000000
stop = 28200000000000
# stop - start = kgV(rtts)

for initialMaxDataSize in initialMaxDataSizes:
	gps_measured[initialMaxDataSize] = []
	for rtt in rtts:
		file = path + "initialMaxData=" + str(initialMaxDataSize) +"kB/delay=" + str(int(rtt/2)) +"/0.vec"
		print(file)
		conn = None
		try:
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()

		c.execute("""
		select sum(value)
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'bottleneck_link.receiver.quic' and vectorName = 'streamRcvDataBytes_cid=0_sid=0:vector'
		) and """ + str(start) + " <= simtimeRaw and simtimeRaw < " + str(stop))
		receivedBytes = c.fetchone()[0]
		c.close()
		conn.close()

		print(receivedBytes)
		gp = (receivedBytes*8000000 / (stop - start))
		print(gp)
		print("should be :" + str((initialMaxDataSize*8)/rtt))
		gps_measured[initialMaxDataSize].append(gp)

result = {
	'initialMaxDataSizes': initialMaxDataSizes,
	'rtts': rtts,
	'gps': gps_measured
}
np.save("validation", result, allow_pickle=True)
