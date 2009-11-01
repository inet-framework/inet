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
x1, y1 = io.loadtxt(base + "_n1.dly", usecols=[0,1], unpack=True)
x2, y2 = io.loadtxt(base + "_n10.dly", usecols=[0,1], unpack=True)
x3, y3 = io.loadtxt(base + "_n100.dly", usecols=[0,1], unpack=True)
x4, y4 = io.loadtxt(base + "_n400.dly", usecols=[0,1], unpack=True)

plt.close()
l1, l2, l3, l4 = plt.plot(x1, y1, 'bo-', x2, y2, 'r*-', x3, y3, 'gD-', x4, y4, 'c^-', linewidth=3)
plt.legend((l1, l2, l3, l4), ('$n$=1', '$n$=10', '$n$=100', '$n$=400'), 'center right')
plt.axis([0, 10, 0, 100])
plt.xlabel("Access Rate ($R_D$=$R_F$) [Mbps]")
plt.ylabel("$D_W$ [s]")
plt.xticks(ma.arange(0, 11))
#plt.yticks(ma.arange(0, 50, 10))
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base + ".dly.png", format='png')

