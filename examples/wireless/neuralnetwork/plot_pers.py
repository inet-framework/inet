
import pandas as pd

n = pd.read_csv('pers_n.csv')
p = pd.read_csv('pers_p.csv')
s = pd.read_csv('pers_s.csv')

import matplotlib.pyplot as plt

for (name, xdata) in [('Neural Network', n), ('Packet Level', p), ('Symbol Level', s)]:

    fig = plt.figure()
    ax = plt.gca()
    ax.scatter(xdata, s, label="Symbol Level")
    ax.scatter(xdata, p, label="Packet Level")
    ax.scatter(xdata, n, label="Neural Network")

    ax.set_xlabel(name)
    ax.grid()
    ax.legend()

plt.show()


