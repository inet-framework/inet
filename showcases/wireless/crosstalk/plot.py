
f = open("snirs.txt", "rt")

import numpy as np
import matplotlib.pyplot as plt



for l in f.readlines():
    if l.startswith("[INFO]"):
        l = l[7:]

    if "--" not in l and not l.startswith("Computing"):
        comps = l.split(", ")[:-1]
        nums = [float(n.strip()) for n in comps if n.strip() and n != '\n']

        nums = [0 if np.isnan(n) else n for n in nums]
        for n in nums:
            if np.isnan(n):
                print("NaN")
                print(n, end=", ")
        print("nums:", len(nums))
        print(nums)

        if (len(nums) % 52) != 0:
            print("bad length:", len(nums))
            continue

        _x = np.arange(52)
        _y = np.arange(len(nums)/52)
        _xx, _yy = np.meshgrid(_x, _y)
        x, y = _xx.ravel(), _yy.ravel()


        fig = plt.figure(figsize=(8, 3))
        ax1 = fig.add_subplot(111, projection='3d')
        ax1.scatter(x, y,nums)

        plt.show()