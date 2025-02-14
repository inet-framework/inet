import matplotlib.pyplot as plt
import numpy as np
import os
from collections import Counter

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load(path+"lossylink_fixed.npy", allow_pickle=True).item()
packetLossRates = result['packetLossRates']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
msgLengths = result['msgLengths']
pktPerSecs = result['pktPerSecs']
measurements = result['measurements']
runs = 100

fig, ax = plt.subplots(1, 1, figsize=(7.2, 2.8), dpi=200, tight_layout=True)

#pktPerSecs = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 1000, 10000]
pktPerSecs = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
ns = [1, 2, 3, 4]
packetLossRatesShow = 0.01
reducePacketTimeThresholdsShow = -2
msgLengthsShow = '3000B'

nCounts = {}
for n in ns:
	nCounts[n] = []
for pktPerSecsIndex in range(len(pktPerSecs)):
	for n in ns:
		nCounts[n].append(sum(x >= n for x in measurements[packetLossRatesShow][reducePacketTimeThresholdsShow][msgLengthsShow][0][pktPerSecsIndex]) / runs)

#ts = [0, 0.002, 0.02, 0.2]
#ts = [0.0, 0.001, 0.050, 0.200]
ts = [0.0, 0.020, 0.040, 0.060]
tCounts = {}
for t in ts:
	tCounts[t] = []
for pktPerSecsIndex in range(len(pktPerSecs)):
	#print(measurements[packetLossRatesShow][reducePacketTimeThresholdsShow][1][pktPerSecsIndex])
	for t in ts:
		if t == 0.0:
			tCounts[t].append(1.0)
		else:
			tCounts[t].append(sum(x >= t for x in measurements[packetLossRatesShow][reducePacketTimeThresholdsShow][msgLengthsShow][1][pktPerSecsIndex]) / runs)

print('ns')
for n in ns:
	print(nCounts[n])		
print('ts')
for t in ts:
	print(tCounts[t])

nColor = ['b', 'g', 'r', 'y']
tLinestyle = ['-', '--', ':', '-.']
nIndex = 0
for n in ns:
	tIndex = 0
	for t in ts:
		if n == 1 and t != 0: continue
		allZero = True
		nxt = []
		for index in range(len(nCounts[n])):
			nxtValue = nCounts[n][index]*tCounts[t][index]*100
			#print('nxtValue = nCounts['+str(n)+']['+str(index)+']*tCounts['+str(t)+']['+str(index)+']*100 = '+str(nCounts[n][index])+'*'+str(tCounts[t][index])+'*100 = '+str(nxtValue))
			nxt.append(nxtValue)
			if nxtValue > 0: allZero = False
			
		if allZero: break
		#ax.plot(pktPerSecs, nxt, '.-', label='n='+str(n)+', '+str(lastT)+'<=t<='+str(t))
		#ax.plot(pktPerSecs, nxt, '.-', label='n='+str(n)+', t=['+str(int(lastT*1000))+', '+str(int(t*1000))+']ms')
		ax.plot(pktPerSecs, nxt, color=nColor[nIndex], linestyle=tLinestyle[tIndex], marker='.', label='n='+str(n)+', t='+str(int(t*1000))+'ms')
		tIndex += 1
	nIndex += 1
	#print(nCounts[n])
	# Plot the bar graph given by xs and ys on the plane y=k with 80% opacity.
	

ax.set_xlabel('Send Rate [msg/s]')
ax.set_ylabel('False Positive Prob. [%]')

# On the y axis let's only label the discrete values that we have data for.
#ax.set_yticks(yticks)

plt.title('conn time=10s, plr='+str(packetLossRatesShow*100)+'%, r=' + str(reducePacketTimeThresholdsShow) + ', msgLength='+msgLengthsShow)
#ax.set_title('p='+str(packetLossRates[packetLossRateIndex]*100)+'%')
ax.grid()
ax.legend()
#ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xticks([10, 20, 30, 40, 50, 60, 70, 80, 90, 100])
ax.set_yticks([0.001, 0.01, 0.1, 1, 10, 100])
ax.set_yticklabels(['0.001', '0.01', '0.1', '1', '10', '100'])

fig.savefig('plots/lossylink_fixed_'+str(reducePacketTimeThresholdsShow)+'_'+str(packetLossRatesShow)+'_'+msgLengthsShow+'.png')
plt.show()
