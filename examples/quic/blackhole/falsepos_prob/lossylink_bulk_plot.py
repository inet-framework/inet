import matplotlib.pyplot as plt
import numpy as np
import os
from collections import Counter

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

result = np.load(path+"lossylink_bulk.npy", allow_pickle=True).item()
packetLossRates = result['packetLossRates']
reducePacketTimeThresholds = result['reducePacketTimeThresholds']
measurements = result['measurements']
runs = 100
print(measurements)
fig, ax = plt.subplots(1, 1, figsize=(7.2, 2.8), dpi=200, tight_layout=True)

ns = [1, 2, 3, 4]
packetLossRatesShow = 0.02
reducePacketTimeThresholdsShow = -1

nCounts = {}
for n in ns:
	nCounts[n] = (sum(x >= n for x in measurements[packetLossRatesShow][reducePacketTimeThresholdsShow][0][0]) / runs)

#ts = [0, 0.002, 0.02, 0.2]
#ts = [0.0, 0.001, 0.050, 0.200]
ts = [0.0, 0.020, 0.040, 0.060]
tCounts = {}
for t in ts:
	if t == 0.0:
		tCounts[t] = 1.0
	else:
		tCounts[t] = (sum(x >= t for x in measurements[packetLossRatesShow][reducePacketTimeThresholdsShow][1][0]) / runs)

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
		#allZero = True
		#nxt = []
		#for index in range(len(nCounts[n])):
		nxtValue = nCounts[n]*tCounts[t]*100
			#print('nxtValue = nCounts['+str(n)+']['+str(index)+']*tCounts['+str(t)+']['+str(index)+']*100 = '+str(nCounts[n][index])+'*'+str(tCounts[t][index])+'*100 = '+str(nxtValue))
		#	nxt.append(nxtValue)
		#	if nxtValue > 0: allZero = False
			
		#if allZero: break
		#ax.plot(pktPerSecs, nxt, '.-', label='n='+str(n)+', '+str(lastT)+'<=t<='+str(t))
		#ax.plot(pktPerSecs, nxt, '.-', label='n='+str(n)+', t=['+str(int(lastT*1000))+', '+str(int(t*1000))+']ms')
		#ax.plot(pktPerSecs, nxt, color=nColor[nIndex], linestyle=tLinestyle[tIndex], marker='.', label='n='+str(n)+', t='+str(int(t*1000))+'ms')
		print('ax.bar('+str(nIndex*len(ts)+tIndex)+'), '+str(nxtValue)+')')
		ax.bar(nIndex*len(ts)+tIndex, nxtValue)
		tIndex += 1
	nIndex += 1
	#print(nCounts[n])
	# Plot the bar graph given by xs and ys on the plane y=k with 80% opacity.
	

ax.set_xlabel('Send Rate [msg/s]')
ax.set_ylabel('False Positive Prob. [%]')

# On the y axis let's only label the discrete values that we have data for.
#ax.set_yticks(yticks)

plt.title('conn time=10s, plr='+str(packetLossRatesShow*100)+'%, reducePacketTimeThreshold=' + str(reducePacketTimeThresholdsShow))
#ax.set_title('p='+str(packetLossRates[packetLossRateIndex]*100)+'%')
ax.grid()
ax.legend()
#ax.set_xscale('log')
ax.set_yscale('log')
#ax.set_xticks([10, 20, 30, 40, 50, 60, 70, 80, 90, 100])
ax.set_yticks([0.001, 0.01, 0.1, 1, 10, 100])
ax.set_yticklabels(['0.001', '0.01', '0.1', '1', '10', '100'])

fig.savefig('plots/lossylink_bulk_'+str(reducePacketTimeThresholdsShow)+'_'+str(packetLossRatesShow)+'.png')
plt.show()
