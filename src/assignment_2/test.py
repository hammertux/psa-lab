#!/usr/bin/python3

from matplotlib import pyplot as plt
import numpy as np

hitrates = [
            [22.799014, 19.874873, 20.447761, 20.184031, 19.770731, 20.035357, 19.428066, 19.197540],
            [22.749692, 19.874873, 20.402985, 20.154348, 19.740956, 19.976429, 19.295401, 19.153610],
           ]
x = np.arange(8)
fig = plt.figure()
#ax = fig.add_axes([0,0,1,1])
bar1 = plt.bar(x + 0.00, hitrates[0], color='b', width = 0.25)
bar2 = plt.bar(x + 0.25, hitrates[1], color='g', width = 0.25)
plt.legend(labels=['Snooping OFF', 'Snooping ON'])
plt.ylabel('Hitrate (in % Cache Hits)')
plt.yticks(np.arange(0, 24, 3))
plt.xticks(x + 0.13, ('p0', 'p1', 'p2', 'p3', 'p4', 'p5', 'p6', 'p7'))
plt.xlabel('Processor Number')
plt.title('Comparing How Snooping affects the hitrates on the fft_16_p8.trf')


for bar in bar1:
    plt.text(bar.get_x(), bar.get_height() + 1, bar.get_height(), fontsize=11, bbox=dict(facecolor='b', alpha=0.5))

for bar in bar2:
    plt.text(bar.get_x(), bar.get_height() + 0.3, bar.get_height(), fontsize=11, bbox=dict(facecolor='g', alpha=0.5))


plt.show()

