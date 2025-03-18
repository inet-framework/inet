import math
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/throughput_validation_"

ps = [".008", ".01", ".02"]
rtts_measured = [20, 40, 60]
S = 1252
for p_str in ps:
	p = float(p_str)

	for rtt in rtts_measured:
		file = path + "p="+p_str+"_delay="+str(int(rtt/2))+".vec"
		#print(file)
		conn = None
		try:
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()
		
		start = 4000
		end  = 14000
		
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
) and simtimeRaw between """ + str(start * 1000000000) + " and " + str(end * 1000000000) + """
order by simtimeRaw
"""):
			if first == 0:
				first = row[0]
			else:
				data += int(row[1])
			last = row[0]
			#data += int(row[1])

		c.close()
		conn.close()

		tp = data*8*1000000 / (last - first)
		tp_should = (S*8)/(1000*rtt) * 1/math.sqrt(2*p/3)
		print("p=" + str(p*100) + "%, RTT=" + str(rtt) + "ms: " + "{:.3f}".format(tp) + "Mb/s, should be " + "{:.3f}".format(tp_should) + "Mb/s (" + "{:.2f}".format((1-(tp/tp_should))*100) +"%)")
