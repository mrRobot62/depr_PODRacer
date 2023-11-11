import random
from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

x_vals = []
y_vals = []

index = count()
idx = 0
def animate(i):
    idx = next(index)
    x_vals.append(idx)
    y_vals.append(random.randint(1000,2000))
    plt.cla()
    plt.plot(x_vals, y_vals)
#    plt.xlim(left=max(0, idx-50 ), right=idx+50)
    plt.xlim(left=idx-75, right=idx)
    plt.figure

ani = FuncAnimation(plt.gcf(), animate, interval=100)

plt.tight_layout()
plt.show()

