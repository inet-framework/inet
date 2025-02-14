import numpy as np
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "../../../results/quic/lossy_link/mathis_validation/"

ps = [".001",".0002",".00005"]
rtts_measured = [2, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100]
tps_measured = {}
for p_str in ps:
	p = float(p_str)

	tps_measured[p_str] = []

	for rtt in rtts_measured:
		file = path + "p="+p_str+"/delay="+str(int(rtt/2))+"/0.vec"
		print(file)
		conn = None
		try:
			conn = sqlite3.connect('file:'+file+'?mode=ro', uri=True)
		except sqlite3.OperationalError as e:
			raise RuntimeError('Cannot open '+file) from e
		c = conn.cursor()

		first = 0
		last = 0
		data = 0

		t0 = str( (2 + int(rtt/2)) * 1000000000000 )
		t1 = str( (2 + rtt * 8) * 1000000000000 )
		#if (p == .00005):
		#	t1 = str( (2 + rtt * 4) * 1000000000000 )
		print("measure tp from " + t0 + " to " + t1)
		for row in c.execute("""
	select simtimeRaw, value 
	from vectorData 
	where vectorId = (
		select vectorId 
		from vector 
		where moduleName = 'lossy_link.receiver.quic' and vectorName = 'packetReceived:vector(packetBytes)'
	) and simtimeRaw between """ + t0 + " and " + t1 + """
	order by simtimeRaw
	"""):
			if first == 0:
				first = row[0]
			else:
				data += int(row[1])
			last = row[0]
		c.close()
		conn.close()

		tp = data*8000000 / (last - first)
		print(tp)
		tps_measured[p_str].append(tp)


result = {
	'ps': ps,
	'rtts': rtts_measured,
	'tps': tps_measured
}
np.save("mathis_validation", result, allow_pickle=True)

