import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os

dirname = os.path.dirname(__file__)
dirname += "/" if dirname else ""
path = dirname + "results/"

format = {
	"Up": {
		"color": "b",
		"linestyle": "-"
	},
	"Down": {
		"color": "g",
		"linestyle": (0, (12, 2))
	},
	"OptUp": {
		"color": "r",
		"linestyle": (0, (10, 4))
	},
	"DownUp": {
		"color": "c",
		"linestyle": (0, (8, 6))
	},
	"Binary": {
		"color": "m",
		"linestyle": (0, (6, 8))
	},
	"OptBinary": {
		"color": "y",
		"linestyle": (0, (4, 10))
	},
	"Jump": {
		"color": "k",
		"linestyle": (0, (2, 12))
	}
}
fill_between_alpha=.2

mtus = np.arange(1280, 1501, 4)
algs = [ "Up", "Down", "OptUp", "Binary", "Jump" ]

metrics = np.load(path+"transmission_time.npy")

# Data for plotting
fig, ax = plt.subplots(figsize=(6.4, 2.6), dpi=200, tight_layout=True)

for alg_index in range(len(algs)):
	alg = algs[alg_index]
	ax.plot(mtus, metrics[alg_index], format[alg]["color"], linestyle=format[alg]["linestyle"], label=alg)
	#ax[0].fill_between(mtus, result[alg]["NoPtb"], result[alg]["NoPtbNoTimeout"], color=format[alg]["color"], alpha=fill_between_alpha)
	#ax[1].plot(mtus, result[alg]["Ptb"], format[alg]["color"], linestyle=format[alg]["linestyle"], label=alg)

#fig.set(title='Time (RTT = 20 ms)')
ax.set(xlabel='PMTU [B]', ylabel='Time w/o PTB [s]')
ax.grid()
ax.legend()

#ax[0].set_yscale('log')
#ax[1].set_yscale('log')

fig.savefig("transmission_time.png")
plt.show()


