    
from pylab import *; 
import numpy as np, matplotlib.pyplot as plt, sys; 
import scipy
import torch
from utillc import *
from scipy.signal import butter, lfilter
from scipy.signal import freqs
from scipy.signal import qspline1d, qspline1d_eval
#matplotlib.use('Agg',force=False)
import warnings
warnings.filterwarnings('ignore')
def butter_lowpass(cutOff, fs, order=5):
    nyq = 0.5 * fs
    normalCutoff = cutOff / nyq
    b, a = butter(order, normalCutoff, btype='low', analog = True)
    return b, a

def butter_lowpass_filter(data, cutOff, fs, order=4):
    b, a = butter_lowpass(cutOff, fs, order=order)
    y = lfilter(b, a, data)
    return y

cutOff,fs, order = 50, 100, 5

fin = 'c.txt'

fin = fin.split()

if len(fin) > 0 and fin[0] == '-t' :
    fin = fin[1:]
    t = 'yes'
else :
    t = 'no'

if len(fin) > 1 and fin[0] == '-f' :
    kernel = float(fin[1])
    fin = fin[2:]
else :
    kernel = 5

#EKOX(fin)
if len(fin) > 0 :
    lc = [ '#' + ','.join(fin) ]
    strip = lambda x : x.strip() 
    lines = lambda fn : list(map(strip, open(fn, 'r').readlines()))

    llines = list(map(lines, fin))
    #EKOX(len(list(llines)))
    #EKOX(list(llines)[0])
    #EKOX(len(list(llines)[0]))
    #EKOX(len(list(llines)[1]))
    #print(list(llines)[0][0])
    l = list(map(list, zip(*llines)))
    #EKOX(l[0])
    #EKOX(l[1])
    l = [ list(map(float, l1)) for l1 in l if l1[0][0] != '#']
    #EKOX(l)
else :
    l1 = [ x for x in sys.stdin]
    lc = [ x for x in l1 if len(x) > 0 and x[0] == '#' ]
    ld = [ x for x in l1 if len(x) > 0 and x[0] != '#' ]
    l = [ list(map(float, x.strip().split())) for x in ld]
N=len(l)
EKOX(N)
mat = np.asarray(l)
EKOX(mat.shape)
if t == 'no' :
    t = range(N)
else :
    t = mat[:,0]
    mat = mat[:, 1:]

N,C = mat.shape
print(mat.shape)
try :

    p = lc[0]
    print(('p', lc[0]))
    p = p.strip()
    p = p[1:].split(',')
except :
    p = range(mat.shape[1])

print(33)

kernel_size = kernel
kernel = np.ones(kernel_size) / kernel_size

print(C)
#for i in range(C) :   mat[:,i] = np.convolve(mat[:,i], kernel, mode='same')
#mat = qspline1d_eval(qspline1d(mat), t)
#mat = scipy.signal.medfilt(mat) #, kernel_size=9)
#mat = butter_lowpass_filter(mat, cutOff, fs, order)
print(C)
print(p)
print(('shape', mat.shape))
pp = [ plot(t, mat[:,i], label=p[i-1]) for i in range(0, C)]
plt.legend();
show();
