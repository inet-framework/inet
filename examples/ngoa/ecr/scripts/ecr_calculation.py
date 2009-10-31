#!/usr/bin/env python
#
# IPython script for calculating ECRs of candidate NGOA architecture
# based on web page delay data from OMNeT++ simulation
#
# (C) 2009 Kyeong Soo (Joseph) Kim


import os.path
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import numpy.lib.function_base as num_base
from scipy import interpolate, optimize


######################
### global definitions
######################
# smoothing factor
smoothing = 0

# store the spline representation
tck = ()

# returns a single-argument function which returns interpolated delay given a rate
def f(delay):
    return lambda x: interpolate.splev(x, tck, der=0) - delay


######################
### Main loop
######################

# get file name of delay data from the user
in_name = raw_input("The name of the delay data file: ")

# extract the number of sessions & web page delay to X & Y
X, Y = mlab.load(in_name, usecols=[0,1], unpack=True)

# DEBUG
plt.close()

for i in range(0, len(X)):
    # calculate ECR for a given pair of the number of sessions (n) & web page delay (web_page_delay)
    n = X[i]
    delay = Y[i]
    
    # DEBUG
    #print "for n = %d and delay = %f:" % (n, delay)

    # extract rate & delay to x & y from the data file for ECR reference model
    ecr_basename = "../EcrReferenceWithHttp/N1_br1_bd5_n"
    ecr_name = ecr_basename + str(int(n)) + ".dly"

    # DEBUG
    #print ecr_name

    if (os.path.isfile(ecr_name)):
        # DEBUG
        #print ecr_name
        
        x, y = mlab.load(ecr_name, usecols=[0,1], unpack=True)

        # DEBUG
        tck_dbg = interpolate.splrep(x, y, s=smoothing)
        x_dbg = num_base.linspace(0.01, max(x), 100)
        y_dbg = interpolate.splev(x_dbg, tck_dbg, der=0)
        plt.plot(x, y, 'o', x_dbg, y_dbg, '-')
        plt.show()
    else:
        continue

    # obtain 1-d spline representation of data
    tck = interpolate.splrep(x, y, s=smoothing)

    # obtain ECR by solving f(x)=0 with 10 as a starting point, where f(x) is defined above
    ECR = optimize.newton(f(delay), 0.01)

    # print the result
    result = "ECR = " + repr(ECR) + " for n = " + repr(n)
    print result

    # pause for user input
    raw_input("Press ENTER to continue ...")

    # DEBUG
    plt.clf()
