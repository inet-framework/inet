import os
import numpy as np
from PIL import Image
from watchfiles import watch
import matplotlib.pyplot as plt

def convert_image():
    i = Image.open('snirs.png').convert('L')
    s = np.array(i)
    print(s)
    with open('snirs_img.csv', 'w') as f:
        print(",".join([str(x-128) for x in s.flatten()]), file=f)


fig = plt.figure()
ax = plt.gca()

plt.plot(block=False)
plt.pause(0.1)

def plot_pers():
    n = float(open('pers_n.csv').read())
    p = float(open('pers_p.csv').read())
    s = float(open('pers_s.csv').read())

    h = [n, p, s]
    print(h)
    plt.cla()
    plt.bar(x=0, height=n, label='Neural Network')
    plt.bar(x=1, height=p, label='Packet Level')
    plt.bar(x=2, height=s, label='Symbol Level')

    ax.grid()
    ax.legend()
    plt.plot(block=False)
    plt.pause(0.1)



for changes in watch('snirs.png', yield_on_timeout=True, rust_timeout=100):
    if changes:
        print("image changed")
        convert_image()
        os.system("python3 evaluate-error-models.py -n -p -s evaluate-ieee80211radio-error-models.ini")
        plot_pers()
    else:
        print("painting")
        plt.pause(0.1)

    if not plt.fignum_exists(fig.number):
        break
