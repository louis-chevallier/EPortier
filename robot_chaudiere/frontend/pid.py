from guizero import App, TextBox, Text, Slider, CheckBox
from utillc import *
from datetime import datetime, timedelta, time
import numpy as np
import matplotlib.pyplot as plt

app = App()


class Maison :
    """
    simule la maison : inertie thermique ...
    """
    def __init__(self, app, Ts=1) :
        self.T = Ts # le timestep en sec
        self.date = datetime.now(); # date (sec)
        self.temp_chaudiere = 10 # temp chaudiere en °C
        self.temp_maison = 20 # temp chaudiere en °C
        self.temp_ext = 15 # °
        self.thermostat = 50
        self.thermostat_t = Text(app, text="thermostat=%f" % self.thermostat)        
        self.bruleur = CheckBox(app, text="bruleur")
        self.temp_chaudiere_t = Text(app, text="")
        self.temp_maison_t = Text(app, text="")
        self.temp_ext_t = Text(app, text="")
        self.l = []
        self.ticks = []

        self.state = 0

    def step(self) :
        """
        avance le temps de T sec
        """

        T = self.T


        Q1 = 1./2000 # quantité d'energie passant de l'eau de la chaudiere vers la maison par sec par °
        Q2 = 1./2000 # quantité d'energie passant de la maison vers l'extérieur par sec par °
        
        # chaudiere
        p = 15. / (10 * 60) # puissance chaudiere : augmente de 5° en 10mn (observé)
        #EKOX(self.temp_chaudiere)

        hysteresis = 3

        self.bruleur.bg = "red" if self.state == 1 else "grey"        
        if self.state == 0 :
            if self.thermostat > self.temp_chaudiere + hysteresis :
                self.state = 1
        else :
            self.temp_chaudiere += (p * T) # le bruleur crache
            if self.thermostat < self.temp_chaudiere - hysteresis :
                self.state = 0
            

        cal1 = (self.temp_chaudiere - self.temp_maison) * Q1 * T
        self.temp_maison += cal1
        self.temp_chaudiere -= cal1

        cal2 = (self.temp_maison - self.temp_ext) * Q2 * T
        self.temp_maison -= cal2

        self.temp_chaudiere_t.value = "chaudiere %f" % self.temp_chaudiere
        self.thermostat_t.value = "thermostat %f" % self.thermostat        
        self.temp_maison_t.value = "maison %f" % self.temp_maison
        self.temp_ext_t.value = "ext %f" % self.temp_ext

        self.l += [ (self.thermostat, self.temp_chaudiere,  self.temp_maison,  self.temp_ext)]

        self.date += timedelta(seconds=T)
        self.ticks.append(self.date)
        
            
    def set_temp_chaudiere(self, temp) :
        """
        change le thermostat de la chaudiere
        """
        self.thermostat = temp
        pass
        

m = Text(app, text="PID\n")

maison = Maison(app, Ts=1)



class PID :
    def __init__(self, T) :
        self.T = T

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
m.repeat(Tms, callback)

app.display()

if True :
    #for t in range(10000) :         maison.step()

    l = np.asarray(maison.l)
    EKOX(l.shape)
    lb = [ "thermostat", "chaudiere", "maison", "exterieur"]
    for j in range(4) :
        plt.plot(maison.ticks, l[:,j], label=lb[j])    
    plt.legend(loc="lower right")
    plt.show()
