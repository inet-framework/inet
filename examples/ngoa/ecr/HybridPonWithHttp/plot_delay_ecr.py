#!/usr/bin/env python
#
# IPython script for plotting web page delays and ECRs for hybrid PON
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
base1 = "N16_dr10"
base2 = "br1_bd5"

# plot web page delay
x1, y1 = io.loadtxt("../TdmPonWithHttp/N16_dr10_fr10_br1_bd5.dly", usecols=[0,1], unpack=True)
x2, y2 = io.loadtxt(base1 + "_fr20_" + base2 + ".dly", usecols=[0,1], unpack=True)
x3, y3 = io.loadtxt(base1 + "_fr30_" + base2 + ".dly", usecols=[0,1], unpack=True)
x4, y4 = io.loadtxt(base1 + "_fr40_" + base2 + ".dly", usecols=[0,1], unpack=True)

plt.close()
l1, l2, l3, l4 = plt.plot(x1, y1, 'bo-', x2, y2, 'r*-', x3, y3, 'gD-', x4, y4, 'c^-', linewidth=3)
plt.legend((l1, l2, l3, l4), ('$R_F = R_D$', '$R_F = 2 R_D$', '$R_F = 3 R_D$', '$R_F = 4 R_D$'), 'lower right')
#plt.axis([0, 10, 0, 100])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("$D_W$ [s]")
#plt.xticks(ma.arange(0, 11))
#plt.yticks(ma.arange(0, 50, 10))
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base1 + "_" + base2 + ".dly.png", format='png')

raw_input("Press ENTER to continue ...")

# plot ECR
x1, y1 = io.loadtxt("../TdmPonWithHttp/N16_dr10_fr10_br1_bd5.ecr", usecols=[0,1], unpack=True)
x2, y2 = io.loadtxt(base1 + "_fr20_" + base2 + ".ecr", usecols=[0,1], unpack=True)
x3, y3 = io.loadtxt(base1 + "_fr30_" + base2 + ".ecr", usecols=[0,1], unpack=True)
x4, y4 = io.loadtxt(base1 + "_fr40_" + base2 + ".ecr", usecols=[0,1], unpack=True)

# 1st figure with original range
plt.clf()
l1, l2, l3, l4 = plt.plot(x1, y1, 'bo-', x2, y2, 'r*-', x3, y3, 'gD-', x4, y4, 'c^-', linewidth=3)
plt.legend((l1, l2, l3, l4), ('$R_F = R_D$', '$R_F = 2 R_D$', '$R_F = 3 R_D$', '$R_F = 4 R_D$'), 'upper right')
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("ECR [Mbps]")
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base1 + "_" + base2 + ".ecr1.png", format='png')

raw_input("Press ENTER to continue ...")

#2nd figure with reduced range
plt.clf()
l1, l2, l3, l4 = plt.plot(x1, y1, 'bo-', x2, y2, 'r*-', x3, y3, 'gD-', x4, y4, 'c^-', linewidth=3)
plt.legend((l1, l2, l3, l4), ('$R_F = R_D$', '$R_F = 2 R_D$', '$R_F = 3 R_D$', '$R_F = 4 R_D$'), 'upper right')
plt.axis([0, 100, 0, 10])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("ECR [Mbps]")
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base1 + "_" + base2 + ".ecr2.png", format='png')

