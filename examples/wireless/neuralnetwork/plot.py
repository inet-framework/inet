import numpy as np
import matplotlib.pyplot as plt

def plot_snirs(title, snirs):

    # setup the figure and axes
    fig = plt.figure()
    fig.set_size_inches(10, 6)
    ax = fig.add_subplot(projection='3d')


    top=snirs
    _x = np.arange(52)
    _y = np.arange(len(snirs) / 52)
    _xx, _yy = np.meshgrid(_x, _y)
    x, y = _xx.ravel(), _yy.ravel()

    bottom = [t - 0.1 for t in top]
    x = [x - 26 for x in x]

    width = depth = 1
    import matplotlib.cm as cm
    cmap = cm.get_cmap('jet_r') # Get desired colormap
    rgba = [cmap(k/20) for k in top]

    top = [0.1 for t in top]

    ax.bar3d(x, y, bottom, width, depth, top, shade=True, color=rgba)
    ax.set_zmargin(0)
    ax.set_zlim([0, None])
    ax.set_title(title)
    ax.set_xlabel("OFDM subcarrier index (frequency)")
    ax.set_ylabel("OFDM symbol (time)")
    ax.set_zlabel("Subcarrier symbol SNIR [dB]")

    ax.set_box_aspect([ub - lb for lb, ub in (getattr(ax, f'get_{a}lim')() for a in 'xyz')])

    plt.show()


import sys

if len(sys.argv) < 2:
    raise RuntimeError("Please specify CSV filename to read")

import math
import csv
with open(sys.argv[1]) as csvfile:
    reader = csv.reader(csvfile)
    for f in reader:
        row = list(f)
        if row[0].startswith('#'):
            continue
        numSymbols = int(row[15])
        print(numSymbols)
        snirs = row[-numSymbols-1:-1]
        snirs = [math.log10(float(x))*10 for x in snirs]
        plot_snirs(f"Packet Error Rate: {f[1]}", snirs)
