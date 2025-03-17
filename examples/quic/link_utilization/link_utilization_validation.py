import sqlite3
import os

def getQuicTp(file):
	#print(file)
	try:
		conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
	except sqlite3.OperationalError as e:
		raise RuntimeError('Cannot open '+file) from e
	
	c = conn.cursor()

	start = 4000000000000
	end   = 5000000000000
	first = 0
	last = 0
	data = 0

	for row in c.execute("""
	select simtimeRaw, value 
	from vectorData 
	where vectorId = (
		select vectorId 
		from vector 
		where moduleName = 'bottleneck.receiver.quic' and vectorName = 'packetReceived:vector(packetBytes)'
	) and simtimeRaw between """ + str(start) + " and " + str(end) + """
	order by simtimeRaw
	"""):
		# between start and end, use the time from the first packet arrived until the last packet arrived
		# sum up all data except from the first packet 
		if first == 0:
			first = row[0]
		else:
			data += int(row[1])
		last = row[0]
		#data += int(row[1])

	c.close()
	conn.close()
	return data*8000000 / (last - first)
	#return data*8000000 / (end - start)


dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

bandwidth = 10 # Mb/s
mtus = ['1280', '1452', '1500', '9000'] # B
tps = []
for mtu in mtus:
	# QUIC packet is 28B smaller than MTU (IP packet minus IP header and minus UDP header)
	# Packet on the wire is 7B larger than MTU (link layer overhead)
	tp_should = (int(mtu)-28)/(int(mtu)+7)*bandwidth
	
	tp = getQuicTp(path + "link_utilization_mtu=" + mtu + ".vec")
	print(str(mtu) + "B: should be " + "{:.3f}".format(tp_should) + "Mb/s, is " + "{:.3f}".format(tp) + "Mb/s (" + "{:.2f}".format((1-(tp/tp_should))*100) +"%)")
	tps.append(getQuicTp(path + "link_utilization_mtu=" + mtu + ".vec"))

