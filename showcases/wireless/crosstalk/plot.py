
f = open("snirs.txt", "rt")

import numpy as np
import matplotlib.pyplot as plt


prevnums = None

num_const = 0
num_frequniform = 0
num_timeuniform = 0
num_other = 0


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

        nums = np.array(nums)

        freqHomogeneous = True

        for td in range(int(len(nums) / 52)):
            slot = nums[td*52:(td+1)*52]
            if not np.all(slot == slot[0]):
                freqHomogeneous = False
                break


        if np.all(nums == nums[0]):
            print("all equal:", nums[0])
            num_const += 1
            continue



        if freqHomogeneous:
            print("freqHomogeneous")
            num_frequniform += 1
            continue

        timeHomogeneous = True

        for sc in range(52):
            slots = int(len(nums) / 52)
            sc_snir = nums[sc::52]
            if not np.all(sc_snir == sc_snir[0]):
                timeHomogeneous = False
                break


        if timeHomogeneous:
            print("timeHomogeneous")
            num_timeuniform += 1
            continue


        num_other += 1


        if prevnums is not None:
            if np.all(nums == prevnums):
                continue

        prevnums = nums.copy()

        nums = np.log10(nums) * 10

        _x = np.arange(52)
        _y = np.arange(len(nums)/52)
        _xx, _yy = np.meshgrid(_x, _y)
        x, y = _xx.ravel(), _yy.ravel()


        fig = plt.figure(figsize=(8, 3))
        ax1 = fig.add_subplot(111, projection='3d')
        #ax1.scatter(x, y,nums)

        #plt.show()

print("constant:", num_const, "freq unif:", num_frequniform, "time unif:", num_timeuniform, "num other:", num_other)
