import numpy as np
import scipy
import matplotlib.pyplot as plt
import math
import PySpice.Logging.Logging as Logging
logger = Logging.setup_logging()

from PySpice.Spice.Netlist import Circuit
from PySpice.Unit import *
from PySpice.Spice.BasicElement import *
from PySpice.Spice.HighLevelElement import *
from PySpice.Spice.Simulation import *
#from OperationalAmplifier import BasicOperationalAmplifier

from scipy.io import wavfile
from utillc import *



samplerate, data = wavfile.read('/mnt/NUC/download/dahlia.wav')
EKON(samplerate)
EKOX(1./samplerate)
EKOX(data)
scale_factor = 1. #10000
data = data / scale_factor
EKOX(np.amin(data))
EKOX(np.amax(data))
duration = 3.  # sec
t = np.arange(0, duration, 1./samplerate)
EKOX(t.shape)
EKOX(data.shape)
data = data[:t.shape[0]] 
EKOX(data.shape)
values = np.vstack((t, data)).T
EKOX(values.shape)

#EKOX(values[:50, :])

circuit = Circuit('NonLineal Load Sim')

'''
V1 A 0 SINE(0 220 50)
D1 0 N001 Def
D2 A N001 Def
D3 N003 A Def
D4 N003 0 Deg
R1 N001 N002 27.5
L1 N002 N003 0.5
.MODEL Def D
'''

if False :
    circuit.SinusoidalVoltageSource('1', 'x', circuit.gnd,
                                    amplitude=10,
                                    frequency=900)
ee, nn = 1, 1
def node() :
    global nn
    nn = nn+1
    return "N" + str(nn)

def el() :
    global ee
    ee = ee+1
    return ee

circuit.include('ada4084.cir')
circuit.include('ad8606.cir')

EKO()
def bessel(circuit, a) :
    #https://www.changpuak.ch/electronics/Bessel_Lowpass_active_24dB.php    
    out, n1, n2, n3, n4, n5 = node(), node(), node(), node(), node(), node()
    R1 = circuit.R(el(), a,     n1,       		1@u_kΩ)    
    R2 = circuit.R(el(), n1,    n2,       		1@u_kΩ)    
    R3 = circuit.R(el(), n2,     n4,      		1@u_kΩ)    
    R4 = circuit.R(el(), n4,     n5,      		1@u_kΩ)    
    C1 = circuit.C(el(), n1,     n3,      		14.684@u_nF)    
    C2 = circuit.C(el(), n2,     circuit.gnd,      	13.479@u_nF)    
    C3 = circuit.C(el(), n4,     out,      		20.213@u_nF)    
    C4 = circuit.C(el(), n5,     circuit.gnd,		7.791@u_nF)    
    #EKOX(dir(circuit))
    X1 = circuit.X(551 + el(), 'AD8606', n2, n3, '+5V','-5V', n3)
    X2 = circuit.X(552 + el(), 'AD8606', n5, out, '+5V','-5V', out)
    return out
    
def RC(circuit) :
    KK=0.3
    R1 = circuit.R(2, 'A',     'out',       KK*0.1@u_kΩ)
    C1 = circuit.C(3, 'out',   circuit.gnd, 1./KK*0.2@u_uF)
    break_frequency = 1 / (2 * math.pi * float(R1.resistance * C1.capacitance))
    EKOX(break_frequency)
    return 'out'

if False :

    circuit.D('1',circuit.gnd,'N001', model='Def')
    circuit.D('2','A','N001',model='Def')
    circuit.D('3','N003','A',model='Def')
    circuit.D('4','N003',circuit.gnd,model='Def')
    circuit.R('1','N001','N002',27.5)
    circuit.L('1','N002','N003',0.5)
    circuit.model('Def', 'D')

circuit.V(111, '+5V', circuit.gnd, 'dc 5' )
circuit.V(222, '-5V', circuit.gnd, 'dc -5' )

if False :
    o1 = bessel(circuit, 'A'); 
    o2 = bessel(circuit, o1)
    out = bessel(circuit, o2)

out = RC(circuit)

EKO();
#print(circuit)
circuit.PieceWiseLinearVoltageSource('1', 'A', circuit.gnd, values = values)

simulator = circuit.simulator()

K=10

EKOX(1./samplerate/K)
step = (1./samplerate/K)
analysis = simulator.transient(step_time=step@u_s,
                               end_time=duration@u_s)
EKOT("transient done")
#EKOX(dir(analysis))
#EKOX(dir(circuit))
#EKOX(analysis.nodes)
EKOX(analysis.elements)
EKOX(duration / step)


def interpolate(s) :
    n = s.shape[0]
    tt = np.linspace(0, duration, n)
    return np.interp(t, tt, s)


in_, out = interpolate(np.asarray(analysis.nodes['a'])), interpolate(np.asarray(analysis[out]))
EKOX(np.var(in_))
EKOX(np.var(out))
EKOX(np.var(values[:,1]))

# en flottant
wavfile.write('speech_filtered.wav', samplerate, out)
wavfile.write('speech_in.wav', samplerate, in_)

EKOX(in_.shape)
EKOX(out.shape)

sample_width = 1./samplerate
L = in_.shape[0]
EKOX(L)
DD = int(0.020 / sample_width)
EKOX(DD)
a = in_[DD:L-DD]
la = a.shape[0]
def f(i) : 
    b = out[i:i+la]
    return -np.mean(np.abs(a - b))
aaa = list(map(f, range(DD*2)))
EKOX(np.argmin(aaa))

EKOX(scipy.__version__)
EKOX(scipy.signal.find_peaks(aaa))
delay = np.abs(np.argmin(aaa) - DD) * sample_width
EKOX(delay)

plt.plot(np.asarray(aaa)); plt.show()


if False :

    current = analysis['a']
    aimax = np.amax(current.data)
    aimin = np.amin(current.data)
    print ('Max Current: ',aimax)
    print ('Min Current: ',aimin)

    figure1 = plt.figure(1, (20, 10))
    #plt.plot(analysis.time, current, '-')
    plt.plot(analysis.time, analysis['out'], '-')
    plt.grid()
    plt.title('Current')
    plt.xlabel('time')
    plt.ylabel('Amps')
    plt.axhline(y=aimax,color='red')
    plt.axhline(y=aimin,color='red')
    yticks, ytlabels =plt.yticks()
    yticks[-1]=aimax
    yticks[-1]=aimin
    plt.yticks(yticks)
    plt.show()
