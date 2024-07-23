from guizero import App, TextBox, Text, Slider, CheckBox
from utillc import *
from datetime import datetime, timedelta, time
#import numpy as np
import matplotlib.pyplot as plt

from scipy.signal import butter, lfilter, freqz
import matplotlib.pyplot as plt
from scipy import stats


import jax
from jax import jit
from jax.lib import xla_bridge
import jax.numpy as np
from jax import grad, jit, vmap

#from jax import random

EKOX(f"{xla_bridge.get_backend().platform=}")
def slow_f(x):
    # Element-wise ops see a large benefit from fusion
    y = x * x + x * 2.0
    return y
x = np.ones((5000, 5000))

using_jax = True

if using_jax :
    key = jax.random.PRNGKey(758493)  # Random seed is explicit in JAX

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.dates import DateFormatter



app = App()

def butter_lowpass(cutoff, fs, order=5):
    return butter(order, cutoff, fs=fs, btype='low', analog=False)

def butter_lowpass_filter(data, cutoff, fs, order=5):
    b, a = butter_lowpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y

timestep = 1 #sec
second = 1
minute = 60 * second
hour = 60*minute
jour = 24*hour
Fmax, Fmin = 1./jour, 1./(10*jour)

fs = 1./timestep
T = 10 * jour # 3600 * 30 # 5.0         # seconds
n = int(T * fs) # total number of samples
EKOX(n)
nc = 7
t = np.linspace(0, T, n, endpoint=False)

def uniform(max, min, shp) :
    if using_jax :
        return jax.random.uniform(key, minval=min, maxval=max, shape=shp)
    else :
        return np.random.uniform(low=min, high=max, size=shp)

Fa = uniform(Fmax, Fmin, (nc, 1))
pa = uniform(Fmax, Fmin, (nc, 1))
aa = uniform(0.5, 1, (nc, 1))
EKOX(t)



data = (np.sin(Fa * 2*np.pi * t) + 1) / 2 * 20 * aa / nc * 2
EKOX(data.shape)
data = np.sum(data, axis=0)
EKOX(data.shape)

"""
plt.subplot(2, 1, 2)
plt.plot(t, data, 'b-', label='data')
plt.xlabel('Time [sec]')
plt.grid()
plt.legend()
plt.subplots_adjust(hspace=0.35)
plt.show()
"""


class Maison :
    """
    simule la maison : inertie thermique ...
    """
    def __init__(self, app : App, Ts : float = 1) :
        self.T = Ts # le timestep en sec
        self.date = datetime.now(); # date (sec)
        self.temp_chaudiere = 10 # temp chaudiere en °C
        self.temp_maison = 15 # temp chaudiere en °C
        self.temp_ext = 15 # °

        # thermostat interne a la chaudiere
        # controle le bruleur        
        self.thermostat = 50



        self.thermostat_t = Text(app, text="thermostat=%f" % self.thermostat)        

        self.bruleur = CheckBox(app, text="bruleur")
        self.temp_chaudiere_t = Text(app, text="")
        self.temp_maison_t = Text(app, text="")
        self.temp_ext_t = Text(app, text="")

        # consigne pour le PID
        # le PID va controller le thermostat
        self.consigne = 19.
        self.l = []
        self.ticks = []

        self.state = 0

    def step(self) :
        """
        avance le temps de T sec
        """

        T = self.T


        Q1 = 1./4000 # quantité d'energie passant de l'eau de la chaudiere vers la maison par sec par °
        Q2 = 1./2000 # quantité d'energie passant de la maison vers l'extérieur par sec par °
        
        # chaudiere
        p = 35. / (10 * 60 * second) # puissance chaudiere : augmente de 5° en 10mn (observé)
        #EKOX(self.temp_chaudiere)

        hysteresis = 10

        self.bruleur.bg = "red" if self.state == 1 else "grey"        
        if self.state == 0 :
            if self.thermostat > self.temp_chaudiere + hysteresis :
                self.state = 1
        else :
            self.temp_chaudiere += (p * T) # le bruleur crache
            if self.thermostat < self.temp_chaudiere - hysteresis :
                self.state = 0

        cal1 = (self.temp_chaudiere - self.temp_maison) * Q1 * T
        self.temp_maison += cal1 / 4 / 2
        self.temp_chaudiere -= cal1 / 1

        cal2 = (self.temp_maison - self.temp_ext) * Q2 * T
        self.temp_maison -= cal2 / 6

        self.temp_chaudiere_t.value = "chaudiere %f" % self.temp_chaudiere
        self.thermostat_t.value = "thermostat %f" % self.thermostat        
        self.temp_maison_t.value = "maison %f" % self.temp_maison
        self.temp_ext_t.value = "ext %f" % self.temp_ext

        self.l += [ (self.thermostat, self.temp_chaudiere,  self.temp_maison,  self.temp_ext, self.consigne)]

        self.date += timedelta(seconds=T)
        self.ticks.append(self.date)
        
            
    def set_temp_chaudiere(self, temp) :
        """
        change le thermostat de la chaudiere
        """
        self.thermostat = temp
        pass
        

m = Text(app, text="PID\n")

maison:Maison = Maison(app, Ts=timestep)


class CPID :

    """
    If it overshoots a lot and oscillates, either the integral gain (I) needs to be increased or all gains (P,I,D) should be reduced
    Too much overshoot? Increase D, decrease P.
    Response too damped? Increase P.
    Ramps up quickly to a value below target value and then slows down as it approaches target value? Try increasing the I constant    
"""

    def __init__(self) :
        self.max_len = hour / timestep
        self.ie = [0] * int(self.max_len)
        self.cpt = 0

        pass


    def next(self, mesure, consigne, t) :
        Kp, Ki, Kd = 10, 10, 1./10
        Kp, Ki, Kd = 5, 40, 5
        Kp, Ki, Kd = 3/4, 40, 15*2              
        Kp, Ki, Kd = 3/4/2/10, 40, 15*2*2*10

        Kp, Ki, Kd = 3/4/2/10/5, 100, 15*2*2*10*5
        #Kp, Ki, Kd = [ e * 0.8 for e in (Kp, Ki, Kd)]
        

        
        D = len(self.ie)
        error = consigne - mesure

        #X = np.linspace(-D * timestep, 0, D)        
        #slope, _ = np.polyfit(X, self.ie, deg=1)

        slope = (self.ie[-1] - self.ie[-2]) / timestep
        
        self.ie.append(error)        
        u = Kp * error + Ki * sum(self.ie) / D + Kd * slope
        if len(self.ie) > self.max_len : self.ie.pop(0)
        self.cpt += 1
        """
        if (self.cpt > 4000 and self.cpt < 4100) :
            #EKOX(slope)
            #EKOX(u)
            pass
            EKOX(slope)
            EKOX(u)
            EKOX((self.ie[-1] - self.ie[-2]) / timestep)

        if (t % hour == 0) :
            EKOX(slope)
            EKOX(u)
            EKOX((self.ie[-1] - self.ie[-2]) / timestep)

        return u


pid = CPID()
    
Text(app, text="temp maison")
sliderMeasurement = Slider(app,
                           start=0, end = 50,
                           width=500)


Text(app, text="thermostat")

def thermostat_f(x) :
    maison.thermostat = float(x)

sliderThermostat = Slider(app, width=500,
                          start=10, end = 70,
                          command = thermostat_f)

Text(app, text="temp exterieure")
def temp_ext_f(x) : maison.temp_ext = float(x)
sliderExt = Slider(app, width=500,
                   start=0, end = 30,
                   command = temp_ext_f)

def callback() :
    maison.step()
    sliderMeasurement.value = maison.temp_maison
    
Tms=10 # ms


if False :
    maison.temp_chaudiere = 40
    maison.temp_ext = 17
    maison.consigne = 22
    maison.temp_maison = 23
    maison.thermostat = 60
    for t in range(hour*3) :
        maison.thermostat =  60 if t < 6*minute else 45
        maison.step()

use_pid = True


def run() :
    for t in range(hour*48) :
        maison.step()

        maison.temp_ext = 20 if t < 24*hour else 6
        #maison.temp_ext = 15
        
        if use_pid :
            maison.consigne = 18 if t < 6*hour else 22
            maison.consigne = 19
            v = pid.next(maison.temp_maison, maison.consigne, t)
            th =  maison.thermostat
            maison.thermostat = min(80, max(float(v), 0))
        else :
            maison.thermostat = 50

if True :
    EKO()
    run()
    EKO()
            
if False :
    m.repeat(Tms, callback)
    EKOT("run gui")
    app.display()

if True :
    #for t in range(10000) :         maison.step()
    date_form = DateFormatter("%d-%Hh%Mmn")
    fig, ax = plt.subplots(figsize=(12, 12))
    
    ax.xaxis.set_major_formatter(date_form)
    l = np.asarray(maison.l)
    EKOX(l.shape)
    lb = [ "thermostat", "chaudiere", "maison", "exterieur", "consigne"]
    for j, lbb in enumerate(lb) :
        ax.plot(maison.ticks, l[:,j], label=lbb)    
    plt.legend(loc="lower right")
    plt.show()
