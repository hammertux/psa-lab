#!/usr/bin/python3

from matplotlib import pyplot as plt
import numpy as np


ways = ['Directly-Mapped', '2-way', '4-way', '8-way', '16-way', '32-way', '64-way', 'Fully-Associative']
y_pos = [i for i, _ in enumerate(ways)]
hitrate = [10.806762, 
           11.901802, 
           13.169824, 
           14.335910, 
           14.857174,
           15.182288,
           15.644089,
           18.086692]

bars = plt.bar(y_pos, hitrate, align='center')
plt.xticks(y_pos, ways)
plt.ylabel('Hitrate (% Cache Hits)')
plt.xlabel('N-Way Set Associative Mapping')
plt.title('A Comparison of Hitrates Among Different N-Way Set Associative Caches')

for bar in bars:
    plt.text(bar.get_x(), bar.get_height() + 0.025, bar.get_height())

plt.show()