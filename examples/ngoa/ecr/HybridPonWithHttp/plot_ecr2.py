#!/usr/bin/env python
#
# IPython script for plotting modified ECRs and their spline curves for hybrid PON
#
# (C) 2009 Kyeong Soo (Joseph) Kim


import os.path
import matplotlib.pyplot as plt
import numpy as np
import numpy.lib.io as io
import numpy.core.multiarray as ma
import numpy.lib.function_base as num_base
from scipy import interpolate, optimize


######################
### Main body
######################

# base name of files
base1 = "N16_dr10"
base2 = "br1_bd5"

# get data
plt.close()
x1, y1 = io.loadtxt("../TdmPonWithHttp/N16_dr10_fr10_br1_bd5.ecr", usecols=[0,1], unpack=True)
x2, y2 = io.loadtxt(base1 + "_fr20_" + base2 + ".ecr.new", usecols=[0,1], unpack=True)
x3, y3 = io.loadtxt(base1 + "_fr30_" + base2 + ".ecr.new", usecols=[0,1], unpack=True)
x4, y4 = io.loadtxt(base1 + "_fr40_" + base2 + ".ecr.new", usecols=[0,1], unpack=True)

# # obain 1-d spline curves
# smoothing = 2
# 
# tck = interpolate.splrep(x1, y1, s=smoothing)
# x1_new = num_base.linspace(0.01, 100, 100)
# y1_new = interpolate.splev(x1_new, tck, der=0)
# 
# tck = interpolate.splrep(x2, y2, s=smoothing)
# x2_new = num_base.linspace(0.01, 100, 100)
# y2_new = interpolate.splev(x2_new, tck, der=0)
# 
# tck = interpolate.splrep(x3, y3, s=smoothing)
# x3_new = num_base.linspace(0.01, 100, 100)
# y3_new = interpolate.splev(x3_new, tck, der=0)
# 
# tck = interpolate.splrep(x4, y4, s=smoothing)
# x4_new = num_base.linspace(0.01, 100, 100)
# y4_new = interpolate.splev(x4_new, tck, der=0)

# obtain exponential function through lease square fit

def erfc(p, x, y):
     return y-(p[0]*np.exp(p[1]*x)+p[2]) 

def f(x, p):
    return p[0]*np.exp(p[1]*x)+p[2] 

p0 = [1, -1, 1]

plsq=optimize.leastsq(erfc, p0, args=(x1, y1))
x1_new = num_base.linspace(0.01, 100, 800)
y1_new = f(x1_new, plsq[0])

plsq=optimize.leastsq(erfc, p0, args=(x2, y2))
x2_new = num_base.linspace(0.01, 100, 800)
y2_new = f(x2_new, plsq[0])

plsq=optimize.leastsq(erfc, p0, args=(x3, y3))
x3_new = num_base.linspace(0.01, 100, 800)
y3_new = f(x3_new, plsq[0])

plsq=optimize.leastsq(erfc, p0, args=(x4, y4))
x4_new = num_base.linspace(0.01, 100, 800)
y4_new = f(x4_new, plsq[0])

# plot ECR with reduced range
plt.clf()
l1, l2, l3, l4 = plt.plot(x1, y1, 'bo', x2, y2, 'r*', x3, y3, 'gD', x4, y4, 'c^')
plt.plot(x1_new, y1_new, 'b-', x2_new, y2_new, 'r-', x3_new, y3_new, 'g-', x4_new, y4_new, 'c-', linewidth=3)
plt.legend((l1, l2, l3, l4), ('$R_F = R_D$', '$R_F = 2 R_D$', '$R_F = 3 R_D$', '$R_F = 4 R_D$'), 'upper right')
plt.axis([0, 100, 0, 10])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("ECR [Mbps]")
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base1 + "_" + base2 + ".ecr.new.png", format='png')

raw_input("Press ENTER to continue ...")

# plot multiplication factor to achieve ECR of 10 Mbps
mf = ma.zeros(len(x1_new))
for i in range(len(x1_new)):
    if y1_new[i] >= 10:
        mf[i] = 1
    elif y2_new[i] >= 10:
        mf[i] = 2
    elif y3_new[i] >= 10:
        mf[i] = 3
    elif y4_new[i] >= 10:
        mf[i] = 4
    else:
        mf[i] = 0

plt.clf()
plt.plot(x1_new, mf, linewidth=3)
plt.axis([0, 19, 0, 5])
plt.xlabel("Number of Sessions per ONU ($n$)")
plt.ylabel("$R_F / R_D$")
plt.grid(linestyle='-')
plt.minorticks_on()
plt.show()
plt.savefig(base1 + "_" + base2 + ".ecr.mf.png", format='png')

