import matplotlib.pyplot as plt
import numpy as np
import os

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "cmr12"
})

result = np.load("mathis_validation.npy", allow_pickle=True).item()

rtts = np.arange(0.1, 104.0, 0.01)
rtts_measured = result['rtts']
ps = result['ps']
tps = result['tps']

S = 1252

colors = ["b", "r", "g"]
plt.figure(figsize=(6.4, 4.8), dpi=200, tight_layout=True)
for index in range(len(ps)):
	p_str = ps[index]
	p = float(p_str)

	tp_mathis = (S*8)/(1000*rtts) * 1/np.sqrt(2*p/3)
	plt.plot(rtts, tp_mathis, colors[index], label="loss-rate: " + str(p*100) + " %")

	b = 2
	tp_quic = 3/4 * (S*8)/(1000*rtts) * (np.sqrt(8/(3*p)+b*b)-b)
	#plt.plot(rtts, tp_quic, label="loss-rate=" + str(p*100) + "%")

	#tps_measured = np.load(path+"quicMathis-p=" + p_str + ".npy")
	plt.plot(rtts_measured, tps[p_str], colors[index]+".")

plt.axis([0, 102, 0, 250])
plt.xlabel('RTT [ms]')
plt.ylabel('Throughput [Mbit/s]')
plt.legend()
plt.grid()
#plt.show()
plt.savefig("mathis_validation.pdf")
