import numpy as np
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
duration = 3. # sec
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

EKO()
def bessel(circuit) :
    #https://www.changpuak.ch/electronics/Bessel_Lowpass_active_24dB.php    
    circuit.include('ada4084.cir')
    circuit.include('ad8606.cir')
    R1 = circuit.R(1, 'A',     'n1',       		1@u_kΩ)    
    R2 = circuit.R(2, 'n1',    'n2',       		1@u_kΩ)    
    R3 = circuit.R(3, 'n2',     'n4',      		1@u_kΩ)    
    R4 = circuit.R(4, 'n4',     'n5',      		1@u_kΩ)    
    C1 = circuit.C(5, 'n1',     'n3',      		14.684@u_nF)    
    C2 = circuit.C(6, 'n2',     circuit.gnd,      	13.479@u_nF)    
    C3 = circuit.C(7, 'n4',     'out',      		20.213@u_nF)    
    C4 = circuit.C(8, 'n5',     circuit.gnd,		7.791@u_nF)    
    #EKOX(dir(circuit))
    X1 = circuit.X(551, 'AD8606', 'n2', 'n3', '+5V','-5V', 'n3')
    X2 = circuit.X(552, 'AD8606', 'n5', 'out', '+5V','-5V', 'out')
    circuit.V(111, '+5V', circuit.gnd, 'dc 5' )
    circuit.V(222, '-5V', circuit.gnd, 'dc -5' )
    
def RC(circuit) :
    R1 = circuit.R(2, 'A',     'out',       0.4@u_kΩ)
    C1 = circuit.C(3, 'out',   circuit.gnd, 0.5@u_uF)
    break_frequency = 1 / (2 * math.pi * float(R1.resistance * C1.capacitance))
    EKOX(break_frequency)
    circuit.V(111, '+5V', circuit.gnd, 'dc 5' )
    circuit.V(222, '-5V', circuit.gnd, 'dc -5' )

if False :

    circuit.D('1',circuit.gnd,'N001', model='Def')
    circuit.D('2','A','N001',model='Def')
    circuit.D('3','N003','A',model='Def')
    circuit.D('4','N003',circuit.gnd,model='Def')
    circuit.R('1','N001','N002',27.5)
    circuit.L('1','N002','N003',0.5)
    circuit.model('Def', 'D')

bessel(circuit)
#RC(circuit)
EKO();
#print(circuit)
circuit.PieceWiseLinearVoltageSource('1', 'A', circuit.gnd, values = values)

simulator = circuit.simulator()

K=10

EKOX(1./samplerate/K)
step = (1./samplerate/K)
analysis = simulator.transient(step_time=step@u_s,
                               end_time=duration@u_s)

#EKOX(dir(analysis))
#EKOX(dir(circuit))
EKOX(analysis.nodes)
EKOX(analysis.elements)
EKOX(duration / step)
EKOX(analysis.nodes['out'].shape)
EKOX(analysis.nodes['a'].shape)


def interpolate(s) :
    n = s.shape[0]
    tt = np.linspace(0, duration, n)
    return np.interp(t, tt, s)


in_, out = interpolate(np.asarray(analysis.nodes['a'])), interpolate(np.asarray(analysis['out']))
EKOX(np.var(in_))
EKOX(np.var(out))
EKOX(np.var(values[:,1]))

# en flottant
wavfile.write('speech_filtered.wav', samplerate, out)
wavfile.write('speech_in.wav', samplerate, in_)

EKOX(in_.shape)
EKOX(out.shape)

EKOX(np.mean(np.abs(in_ - out)))


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
