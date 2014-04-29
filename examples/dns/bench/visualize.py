#!/usr/bin/python
import sys

import numpy as np
from pylab import *

def labelstr(path):
    return os.path.splitext(path)[0]
files = sys.argv
del files[0]
data = []
labels = [] 
for file in files:
    print(file)
    data.append( loadtxt(file))
    labels.append(labelstr(file))
figure(1)
boxplot(data)
xticks(range(0,len(labels)),labels)

