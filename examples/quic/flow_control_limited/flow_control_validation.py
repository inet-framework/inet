import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/flow_control_validation_"

rtts = [10, 20]
initialMaxDataSizes = [65, 100, 140]
start = 3000 * 1000000000
stop  = 4000 * 1000000000

for initialMaxDataSize in initialMaxDataSizes:
	for rtt in rtts:
		file = path + "initialMaxData=" + str(initialMaxDataSize) +"kB_delay=" + str(int(rtt/2)) +".vec"
		#print(file)
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
			where moduleName = 'bottleneck.receiver.quic' and vectorName = 'streamRcvAppData_cid=0_sid=0:vector'
		) and """ + str(start) + " <= simtimeRaw and simtimeRaw < " + str(stop))
		receivedBytes = c.fetchone()[0]
		c.close()
		conn.close()

		#print(receivedBytes)
		gp = (receivedBytes*8000000 / (stop - start))
		gp_should = (initialMaxDataSize*8)/rtt
		print("initialMaxData=" + str(initialMaxDataSize) +"kB, RTT=" + str(rtt) +": " + "{:.3f}".format(gp) + "Mb/s, should be " + "{:.3f}".format(gp_should) + "Mb/s (" + "{:.2f}".format((1-(gp/gp_should))*100) +"%)")

