#!/usr/bin/env python
#
# IPython script for calculating ECRs of candidate NGOA architecture
# based on web page delay data from OMNeT++ simulation
#
# (C) 2009 Kyeong Soo (Joseph) Kim


import os.path
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import numpy.lib.io as io
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
X, Y = io.loadtxt(in_name, usecols=[0,1], unpack=True)

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
        
        # DEBUG
        plt.clf()

        new_smoothing = 0
        while new_smoothing >= 0:
            smoothing = new_smoothing
            x, y = io.loadtxt(ecr_name, usecols=[0,1], unpack=True)

            # DEBUG
            print "Current smoothing factor = %f" % (smoothing)
            tck = interpolate.splrep(x, y, s=smoothing)
            x_new = num_base.linspace(0.01, max(x), 100)
            y_new = interpolate.splev(x_new, tck, der=0)
            plt.plot(x, y, 'o', x_new, y_new, '-')
            plt.show()

            tmp = raw_input("Input new smoothing factor (press ENTER when done): ")
            if (tmp == ""):
                break
            else:
                new_smoothing = float(tmp)
    else:
        continue

    # obtain 1-d spline representation of data
    #tck = interpolate.splrep(x, y, s=smoothing)

    # Before calculating ECR through the interpolated function (i.e., f(x)),
    # check first if the delay is less than the minimum delay of the ECR
    # reference model. If that's the case, its ECR is the line rate of
    # the reference model.
    if (delay <= min(y)):
        ECR = max(x)
    elif (delay > interpolate.splev(0.01, tck, der=0)):
        ECR = 0.01
    else:
        # obtain ECR by solving f(x)=0 with 0.01 as a starting point, where f(x) is defined above
        ECR = optimize.newton(f(delay), 0.01)

    # print the result
    result = "n = " + repr(n) + "\tECR = " + repr(ECR)
    print result

    # pause for user input
    raw_input("Press ENTER to continue ...")

    # DEBUG
    plt.clf()
