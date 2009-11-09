#!/usr/bin/env python
#
# IPython script for plotting web page delays for ECR reference model
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
base = "N1_br1_bd5"

# plot web page delay
x, y = io.loadtxt(base + "_n5.dly.org", usecols=[0,1], unpack=True)

plt.close()
plt.plot(x, y, linewidth=3)
plt.axis([0, 10, 0, 4])
plt.xlabel("Access Rate ($R_D$=$R_F$) [Mbps]")
plt.ylabel("$D_W$ [s]")
plt.xticks(ma.arange(0, 11))
#plt.yticks(ma.arange(0, 50, 10))
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base + "_n5.dly.png", format='png')

