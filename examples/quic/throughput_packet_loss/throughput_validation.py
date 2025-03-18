import math
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/throughput_validation_"

ps = [".008", ".01", ".02"]
rtts_measured = [20, 40, 60]
S = 1252
print("---")
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
		
		print("p=" + str(p*100) + "%, RTT=" + str(rtt) + "ms")
		tp_mathis = (S*8)/(1000*rtt) * 1/math.sqrt(2*p/3)
		print("should be: " + "{:.3f}".format(tp_mathis) + "Mb/s")
		
		#first = 0
		#last = 0
		data = 0

		t0Int = 4000 
		t0 = str( t0Int * 1000000000 )
		t1Int = 14000
		t1 = str( t1Int * 1000000000 )
		for row in c.execute("""
select simtimeRaw, value 
from vectorData 
where vectorId = (
	select vectorId 
	from vector 
	where moduleName = 'bottleneck.receiver.quic' and vectorName = 'packetReceived:vector(packetBytes)'
) and simtimeRaw between """ + t0 + " and " + t1 + """
order by simtimeRaw
"""):
			#if first == 0:
			#	first = row[0]
			#else:
			#	data += int(row[1])
			#last = row[0]
			data += int(row[1])
		
		tp = data*8 / ((t1Int - t0Int)*1000)
		print(str(t0Int) + "ms - " + str(t1Int) + "ms: " + "{:.3f}".format(tp) + "Mb/s (" + "{:.2f}".format((1-(tp/tp_mathis))*100) +"%)")
		c.close()
		conn.close()
		print("---")

