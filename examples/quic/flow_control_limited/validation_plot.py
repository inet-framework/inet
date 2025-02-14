import matplotlib.pyplot as plt
import os
import numpy as np

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("validation.npy", allow_pickle=True).item()

initialMaxDataSizes = result['initialMaxDataSizes']
rtts_measured = result['rtts']
gps = result['gps']

rtts = np.arange(0.001, 110.0, 0.1)
colors = {
	65: "b",
	100: "r",
	140: "g"
}

plt.figure(figsize=(6.4, 4.8), dpi=200, tight_layout=True)
for initialMaxDataSize in initialMaxDataSizes:
	# theory
	gp = initialMaxDataSize*8 / rtts
	plt.plot(rtts, gp, colors[initialMaxDataSize], label="initialMaxData: " + str(initialMaxDataSize) + " kB")
	
	# simulation
	plt.plot(rtts_measured, gps[initialMaxDataSize], colors[initialMaxDataSize]+".")
		
#--------------------------------------------------------------------
plt.xlabel('RTT [ms]')
plt.ylabel('Goodput [Mbit/s]')
plt.axis([0, 102, 0, 120])
plt.legend()
plt.grid()
plt.savefig("validation.pdf")
plt.show()
