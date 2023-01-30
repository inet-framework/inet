import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mt

fig, ax = plt.subplots(subplot_kw={"projection": "3d", "computed_zorder": False})

def read_and_plot(name):
    df = pd.read_csv(name + ".csv", sep=" ", index_col=0, dtype=float)
    df.columns = df.columns.astype(float)

    x,y = np.meshgrid(df.columns, df.index)

    ax.scatter(x, y, np.log10(df) * 10, label=name, s=1, alpha=0.25, depthshade=False)

read_and_plot("noise")
read_and_plot("power")
read_and_plot("snir")

def log_tick_formatter(val, pos=None):
    return f"${{{int(val)}}} dB$"

ax.zaxis.set_major_formatter(mt.FuncFormatter(log_tick_formatter))
ax.zaxis.set_major_locator(mt.MaxNLocator(integer=True))

plt.legend()

plt.show()


