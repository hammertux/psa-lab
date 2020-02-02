#!/usr/bin/python3

from matplotlib import pyplot as plt
import numpy as np

hitrates = [
            [406, 7551],
            [8250, 4538]
           ]
x = np.arange(2)
fig = plt.figure()
bar1 = plt.bar(x + 0.00, hitrates[0], color='b', width = 0.25)
bar2 = plt.bar(x + 0.25, hitrates[1], color='g', width = 0.25)
plt.legend(labels=['Lab 3', 'Lab 2'])
plt.ylabel('Memory Accesses')
plt.yticks(np.arange(0, 10000, 500))
plt.xticks(x + 0.13, ('Reads', 'Writes'))
plt.xlabel('Main Memory Operation')
plt.title('Comparing the memory accesses on the fft_16_p8.trf for Lab 2 VS Lab 3')


for bar in bar1:
    plt.text(bar.get_x(), bar.get_height() + 1, bar.get_height(), fontsize=11, bbox=dict(facecolor='b', alpha=0.5))

for bar in bar2:
    plt.text(bar.get_x(), bar.get_height() + 0.3, bar.get_height(), fontsize=11, bbox=dict(facecolor='g', alpha=0.5))


plt.show()

