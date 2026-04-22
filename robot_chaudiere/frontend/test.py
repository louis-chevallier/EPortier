import cherrypy, os, pickle
from time import sleep
from threading import Thread
from utillc import *
from utillc import EKO, EKOX, EKOT
import utillc
import requests
import time, json
import meteofrance_api
import datetime
import nmap, subprocess, json, re, time


kmh_to_knot = 1.852
boulet = 48.345476, -1.641183

meteo = meteofrance_api.MeteoFranceClient()
#obs = self.meteo.get_observation(48.216671,-1.75) # gps de la meziere
obs = meteo.get_observation(*boulet)

def degre_to_aa(d) :
	q = (360 + 45//2) //8
	i = d//q
	return [ "N", "NE", "E", "SE", "S", "SO", "O", "NO"][i]


EKOX("\n".join(map(str, [ (i, degre_to_aa(i)) for i in range(360)])))

EKOX(obs.wind_speed * kmh_to_knot)
EKOX(obs.wind_direction)
EKOX(degre_to_aa(obs.wind_direction))
forecast = meteo.get_forecast(48.568886594144104, -1.975317996277762).forecast




