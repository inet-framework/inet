import matplotlib.pyplot as plt
import numpy as np
import os

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("shared_link.npy", allow_pickle=True).item()

senders = result['senders']
measurements = result['measurements']

#t0 = 0
#t1 = 200000
dt = 1000
t0 = [20000, 40000, 80000]
t1 = [120000, 160000, 180000]

#t = np.arange(dt/1000/2, int((t1-t0)/dt)+1, 1)
plt.figure(figsize=(7.6, 4.8), dpi=200, tight_layout=True)
for sender in range(senders):
	t = np.arange((t0[sender])/1000, (t1[sender])/1000+2, 1)
	tps = [ measurements[sender][0][i] for i in range(int(t0[sender]/1000)-1, int(t1[sender]/1000)+1) ]
	err = [ measurements[sender][1][i] for i in range(int(t0[sender]/1000)-1, int(t1[sender]/1000)+1) ]
	plt.errorbar(t, tps, fmt=".-", label="Sender" + str(sender+1), yerr=err)
	#print("sender " + str(sender+1))
	#for sampleNum in range(201):
	#	print(str(sampleNum) + ": " + str(tps_avg[sender][sampleNum]))

plt.axis([0, 200, 0, 10])
plt.xlabel('Time [s]')
plt.ylabel('Throughput [Mbit/s]')
plt.legend(loc='upper center')
#plt.xticks(np.arange(5), ('Tom', 'Dick', 'Harry', 'Sally', 'Sue'))
#locs, labels = plt.xticks()
#print(locs)
#print(labels)
#locs.append(2)
#plt.xticks(locs, labels)
#plt.xticks(list(plt.xticks()[0]) + [10, 40, 80, 120, 160, 190])
#plt.xticks([10, 40, 80, 120, 160, 190], ["s1 starts", "s2 starts", "s3 starts", "s1 stops", "s2 stops", "s3 stops"], rotation=20)
plt.xticks(np.arange(0, 201, step=20))
plt.grid()
plt.savefig("shared_link.pdf")
plt.show()
