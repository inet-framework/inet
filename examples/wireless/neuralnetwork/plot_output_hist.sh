# cd results
# cat *.csv | cut -d ',' -f 2 > allpacketrates.csv

# ipython3

# > import matplotlib.pyplot as plt
# > import pandas as pd
# > df = pd.read_csv("allpacketrates.csv")
# > df = df.apply(pd.to_numeric, errors="coerce")
# > df.plot.hist()
# > plt.show()
