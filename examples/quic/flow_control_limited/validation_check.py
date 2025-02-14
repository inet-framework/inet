import sqlite3
import os
#--------------------------------------------------------------------

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

#rtts_measured = [2, 4, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
rtts_measured = [20]
initialMaxData = [65]

for initialMaxDate in initialMaxData:
	for rtt in rtts_measured:
		file = path + "validation_delay=" + str(int(rtt/2)) +"_initialMaxData=" + str(initialMaxDate) +"kB.vec"
		print(file)
		
		lastTime = 0
		data = 0
		times = []
		dataBulks = []
		
		conn = sqlite3.connect(file)
		c = conn.cursor()

		for row in c.execute("""
		select simtimeRaw, value 
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'bottleneck_link.receiver.quic' and vectorName = 'streamRcvDataBytes_cid=0_sid=0:vector'
		)
		order by simtimeRaw
		"""):
			currentTime = row[0]
			if lastTime < currentTime:
				times.append(lastTime)
				dataBulks.append(data)
				data = 0
					
			data += int(row[1])
			lastTime = currentTime
			
		conn.close()
		times.append(lastTime)
		dataBulks.append(data)
		
		for index in range(1, len(dataBulks)):
			print("+" + str(int((times[index]-times[index-1])/1000000000)) + ": " + str(dataBulks[index]) + " - " + str(int((times[index])/1000000000)))
		