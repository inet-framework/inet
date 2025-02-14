import numpy as np
import sqlite3
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

#packetSizes = ['2kB', '20kB', '250kB', '1MB', '2MB', '3MB', '4MB', '5MB', '6MB', '7MB', '8MB', '9MB', '10MB', '60MB']
#packetSizes = ['2kB', '20kB', '250kB', '1', '2', '2200', '2500', '3', '4', '5', '6', '7', '8', '9', '10', '60MB']
#packetSizes = np.concatenate([np.arange(2, 101, 1), np.arange(100, 1001, 100)])
#packetSizes = np.concatenate([np.arange(2, 100, 1), np.arange(100, 2000, 100), np.arange(2000, 10001, 1000)])
#ackFrequencys = ['1','2']

#sizes = np.concatenate([np.arange(2, 10, 1), np.arange(10, 100, 10), np.arange(100, 1000, 100), np.arange(1000, 10001, 1000)])
sizes = [1259, 1585, 1995, 2512, 3162, 3981, 5012, 6310, 7943, 10000, 12589, 15849, 19953, 25119, 31623, 39811, 50119, 63096, 79433, 100000, 125893, 158489, 199526, 251189, 316228, 398107, 501187, 630957, 794328, 1000000, 1258925, 1584893, 1995262, 2511886, 3162278, 3981072, 5011872, 6309573, 7943282, 10000000]
iBits = ['false', 'true']

sizesKB = []
for size in sizes:
	sizesKB.append(size/1000)
	
result = [iBits, sizesKB, []]

for iBitIndex in range(len(iBits)):
	iBit = iBits[iBitIndex]
	
	result[2].append([])
	for size in sizes:
		#print("packet size: "+packetSize+", iBit="+iBit)
		file = path + 'falsepos1-pmtuValidator=true,size='+str(size)+',iBit='+iBit+'.vec'

		conn = sqlite3.connect(file)
		c = conn.cursor()

		c.execute("""
		select simtimeRaw
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'Bottleneck.sender.quic' and vectorName = 'dplpmtudInvalidSignals_cid=0_pid=0:vector'
		)
		order by simtimeRaw asc
		limit 1
		""")
		t1Raw = c.fetchone()[0]
		t1 = t1Raw / (10**12)
		#print(t1)

		c.execute("""
		select simtimeRaw
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'Bottleneck.receiver.app[0]' and vectorName = 'bytesReceived:vector'
		)
		order by simtimeRaw desc
		limit 1
		""")
		t2Raw = c.fetchone()[0]
		t2 = t2Raw / (10**12)
		lastPacketReceivedAtWith = t2

		c.execute("""
		select sum(value) 
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'Bottleneck.receiver.app[0]' and vectorName = 'bytesReceived:vector'
		) and simtimeRaw < """+str(int(t1*10**12)))
		bytesReceived = c.fetchone()[0]

		bpsWith = bytesReceived*8 / (t2-t1)

		#print("w/  PmtuValidator: "+str(bytesReceived)+" bytes received in "+str(bpsWith)+" b/s, "+str(lastPacketReceivedAtWith)+" s")
		c.close()

		file = path + 'falsepos1-pmtuValidator=false,size='+str(size)+',iBit='+iBit+'.vec'

		conn = sqlite3.connect(file)
		c = conn.cursor()
		c.execute("""
		select simtimeRaw
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'Bottleneck.receiver.app[0]' and vectorName = 'bytesReceived:vector'
		)
		order by simtimeRaw desc
		limit 1
		""")
		t2Raw = c.fetchone()[0]
		t2 = t2Raw / (10**12)
		lastPacketReceivedAtWithout = t2

		c.execute("""
		select sum(value) 
		from vectorData 
		where vectorId = (
			select vectorId 
			from vector 
			where moduleName = 'Bottleneck.receiver.app[0]' and vectorName = 'bytesReceived:vector'
		) and simtimeRaw > """+str(int(t1*10**12)))
		bytesReceived = c.fetchone()[0]

		bpsWithout = bytesReceived*8 / (t2-t1)
	
		#print("w/o PmtuValidator: "+str(bytesReceived)+" bytes received in "+str(bpsWithout)+" b/s, "+str(lastPacketReceivedAtWithout)+" s")
		c.close()

		#print("diff: "+str(bpsWithout-bpsWith)+" b/s, "+str(lastPacketReceivedAtWith-lastPacketReceivedAtWithout)+" s")
		#print("ackFrequency="+ackFrequency+", size="+str(packetSize)+": "+str(lastPacketReceivedAtWith-lastPacketReceivedAtWithout)+" s")
		delay = (lastPacketReceivedAtWith - lastPacketReceivedAtWithout) * 1000
		result[2][iBitIndex].append(delay)
	#print("---")

#print(delays)
np.save(path + "falsepos1", result, allow_pickle=True)