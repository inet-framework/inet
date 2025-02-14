#!/usr/bin/env python3.9
import numpy as np
import sqlite3
import matplotlib.pyplot as plt
import os

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "Times"
})

result = np.load("quicTpByMTU.npy", allow_pickle=True).item()

mtus = result['mtus']
tp = result['tp']

plt.figure(figsize=(5.2, 4.8), dpi=200, tight_layout=True)

width = 0.5
offset = 0.1
i = 0
for mtu in mtus:
	max = (int(mtu)-28)/(int(mtu)+7)*100
	print(str(mtu)+": "+str(tp[i])+" of "+str(max))
	if i == 0:
		plt.plot([i-(width/2+offset), i+(width/2+offset)], [max, max], color='red', linestyle='dashed', label='Theoretical maximum')
	else:
		plt.plot([i-(width/2+offset), i+(width/2+offset)], [max, max], color='red', linestyle='dashed')
	i = i+1

xmin, xmax, ymin, ymax = plt.axis()
ymin = 90
ymax = 100
plt.axis([xmin, xmax, ymin, ymax])
plt.bar(mtus, tp, width)
plt.xlabel('MTU (B)')
plt.ylabel('Throughput (Mbit/s)')
plt.legend()

#plt.show()
plt.savefig("quicTpByMTU.pdf")
