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
from scipy.io import wavfile
from utillc import *
samplerate, data = wavfile.read('speech.wav')
EKON(samplerate)




data = data / 10000 
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


#circuit.SinusoidalVoltageSource('1', 'A', circuit.gnd, amplitude=220, frequency=50)
circuit.PieceWiseLinearVoltageSource('1', 'A', circuit.gnd, values = values)

R1 = circuit.R(1, 'A', 'out', 0.2@u_kΩ)
C1 = circuit.C(1, 'out', circuit.gnd, 0.5@u_uF)

if False :

    circuit.D('1',circuit.gnd,'N001', model='Def')
    circuit.D('2','A','N001',model='Def')
    circuit.D('3','N003','A',model='Def')
    circuit.D('4','N003',circuit.gnd,model='Def')
    circuit.R('1','N001','N002',27.5)
    circuit.L('1','N002','N003',0.5)
    circuit.model('Def', 'D')

break_frequency = 1 / (2 * math.pi * float(R1.resistance * C1.capacitance))
EKOX(break_frequency)
#print(circuit)

simulator = circuit.simulator()
analysis = simulator.transient(step_time=1@u_ms, end_time=duration@u_s)

wavfile.write('speech_filtered.wav', samplerate, analysis['out'])

in_, out = np.asarray(analysis['V1']), np.asarray(analysis['out'])

EKOX(np.mean(np.abs(in_ - out)))


current = analysis['V1']
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
