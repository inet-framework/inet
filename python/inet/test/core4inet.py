"""
The end-to-end delays of the medium_network example of Core4INET, a TSN model of another simulation framework:
https://github.com/CoRE-RG/CoRE4INET/tree/master/examples/tsn/medium_network

The validation tests of the TSN asynchronous shaper and credit based shaper compare INET's results with these delays,
and so do the charts of their analysis files. The module imports only pandas, so that a chart script needs no opp_repl.
"""

import pandas as pd

def compute_endtoend_delay():
    df = pd.DataFrame(index=["Medium", "High", "Critical"],
                      data={"min": [88.16, 60.8, 252.8],
                            "max": [540, 307.2, 375.84],
                            "mean": [247.1, 161.19, 298.52],
                            "stddev": [106.53, 73.621, 36.633]})
    df["min"] -= 0.001 # 1 ns initial production offset, see INI file
    df = df + 0.05 * 2 # 50 ns propagation delay per hop
    df.index.set_names(["trafficclass"], inplace=True)
    df.columns.set_names(["name"], inplace=True)
    return df
