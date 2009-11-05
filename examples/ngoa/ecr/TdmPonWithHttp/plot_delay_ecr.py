#!/usr/bin/env python
#
# IPython script for plotting web page delays and ECRs for TDM-PON
#
# (C) 2009 Kyeong Soo (Joseph) Kim


import os.path
import numpy.lib.io as io
import numpy.core.multiarray as ma
import matplotlib.pyplot as plt


######################
### Main body
######################

# base name of files
base = "N16_dr10_fr10_br1_bd5"

# plot web page delay
x, y = io.loadtxt(base + ".dly", usecols=[0,1], unpack=True)
plt.close()
plt.plot(x, y, 'o-', linewidth=3)
plt.axis([0, 100, 0, 40])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("$D_W$ [s]")
plt.yticks(ma.arange(0, 50, 10))
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base + ".dly.png", format='png')

raw_input("Press ENTER to continue ...")

# plot ECR
x, y = io.loadtxt(base + ".ecr", usecols=[0,1], unpack=True)
plt.clf()
plt.plot(x, y, 'o-', linewidth=3)
plt.axis([0, 100, 0, 10])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("ECR [Mbps]")
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base + ".ecr.png", format='png')

