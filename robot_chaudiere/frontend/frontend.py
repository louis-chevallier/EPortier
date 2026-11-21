
import cherrypy, os, pickle
import lzma
from time import sleep
from threading import Thread, Lock
from utillc import *
from utillc import EKO, EKOX, EKOT
import utillc
import requests
import time, json
import meteofrance_api
import datetime
import nmap, subprocess, json, re, time
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np
import nmap, requests
import argparse
from collections import deque

DATA_DIR="/deploy/data"

mylock = Lock()
t11 =  datetime.datetime.now()

SAVE_PERIOD_HOUR, SAVE_PERIOD_MINUTES = 24, 0
SAVE_PERIOD_HOUR, SAVE_PERIOD_MINUTES = 0, 4

def update_t1() :
	with mylock:
		t11 = datetime.datetime.now()

def get_t1() :
	with mylock :
		vt11 = t11
	return vt11

update_t1()

tasks_list = []

class Task(object):

	def data_filename(self) :
		return os.path.join(DATA_DIR, "new_frontend_buffer_%s_%05d.pickle" % ("test" if args.debug else "",  self.interval_sec))
				
	
	def __init__(self, duration_sec, args, max_length=1000):
		"""
		duration_sec : l'etendue temporelle couverte par cette tache
		max_length : le nombre d'échantillons sur cette période
		interval_sec : seconds
		"""
		tasks_list.append(self)
		self.args = args
		self.max_length = max_length
		self.meteo = meteofrance_api.MeteoFranceClient()
		self.interval_sec = max(duration_sec / max_length, 30) # espace minimal entre 2 mesures : 30 sec
		EKON(duration_sec, max_length, self.interval_sec)
		self.buffer = []
		try :
			with open(self.data_filename(), "rb") as fd :
				self.buffer = pickle.load(fd)
				EKON(len(self.buffer), max_length)
				self.filter_buffer()
		except Exception as e:
			EKOX(e)
			with open(self.data_filename(), "wb") as fd :
				pickle.dump(self.buffer, fd, protocol=pickle.HIGHEST_PROTOCOL)
				
			EKOX(e)

		try :
			EKOX(self.buffer[0])
		except :
			pass
		
		self.obs = None
		self.save_time = self.t2 = datetime.datetime.now() - 2 * datetime.timedelta(hours = 24)
		self.delta_check = datetime.timedelta(minutes=30)
		self.check_time = self.t2 = datetime.datetime.now() - self.delta_check

		self.devices = {}
		
		self.thread = Thread(target=self.run, args=())
		self.thread.daemon = True							 # Daemonize thread
		self.thread.start()									 # Start the execution
		if args.debug :
			self.read_plot()
			
	def read_plot(self) :
		with open(self.data_filename(), "rb") as fd :
			self.buffer = pickle.load(fd)
			self.filter_buffer()
		def r(d) :
			def iso(v) :						
				if isinstance(v, datetime.datetime) :
					return v.isoformat()
				elif type(v) is int :
					return datetime.datetime.fromtimestamp(v).isoformat()
				elif type(v) is str and v.isdigit() :
					return datetime.datetime.fromtimestamp(int(v)).isoformat()								
				else :
					return v
						
			def get_date_iso(k) :
				assert('date' in k)
				if k in d :
					return iso(d[k])
				else :
					return iso(d[k + '_iso'])
				
			return (get_date_iso('date'),
					d['direction_vent'],
					get_date_iso('obs_forecast_date'),
					d['obs_forecast_direction_vent'],
					d['date'],
					d['tempchaudiere'])
		def dt(xx) :
			#return datetime.strptime(xx, "%Y-%d-%mT%H:%M:%S.%f")				
			return datetime.datetime.fromisoformat(xx)

		if len(self.buffer) > 1 :
				EKOX(datetime.datetime.now().isoformat())
				EKOX(self.buffer[0])
				EKOX(self.buffer[-1])

				b = [ r(d) for  d in self.buffer]
				dates, temps = [ dt(e[4]) for e in b], [ e[5]['value'] for e in b]
				fig, ax = plt.subplots(figsize=(10, 5))
				ax.plot(dates, temps, linestyle="-", color="#2563eb")
				ax.xaxis.set_major_formatter(mdates.DateFormatter("%d/%m %H:%M"))
				fig.autofmt_xdate()  # Incline les dates pour éviter qu'elles ne se chevauchent
				ax.set_ylabel("Températures")
				ax.grid(True, linestyle="--", alpha=0.6)

				plt.tight_layout()
				plt.show()

				if False :
						ll = [ (dt(aa[0])-dt(bb[0])).total_seconds() for aa,bb in zip(b[1:], b[:-1])]
						EKO()
						plt.plot(ll)
						plt.show()
						x1, y1 = [ e[0] for e in b], [ e[1] for e in b]

						#x1, y1 = np.linspace(0, 12, 1000), np.sin(x1)

						EKON(x1[:10], y1[:10])
						#plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m/%d/%Y'))
						#plt.gca().xaxis.set_major_locator(mdates.DayLocator())
						plt.plot(x1,y1, color='r')
						x2, y2 = [ e[2] for e in b], [ e[3] for e in b]
						#x2, y2 = np.linspace(0, 12, 1000),  np.cos(x2)
						EKON(x2[:10], y2[:10])		

						#plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m/%d/%Y'))
						#plt.gca().xaxis.set_major_locator(mdates.DayLocator())
						plt.plot(x2,y2, color='b')
						plt.show()

				#plt.gcf().autofmt_xdate()
				utillc.default_opt["with_date"] = 1


	def filter_buffer(self) :
		def g(v) :
				return v.isoformat() if isinstance(v, datetime.datetime) else v
		def f(d) : return dict([(k,g(v)) for k,v in d.items()])
		self.buffer = [ f(d) for d in self.buffer]
		
	def discover_nodemcu(self) :
		batcmd="nmap -sL 192.168.1.*"
		result = subprocess.check_output(batcmd, shell=True, text=True)
		trace = self.index() == 0
		trace = False
		result = result.split("\n")
		if trace :
				EKOX(result)

		#if 'salon' in self.devices :			 return

		for e in result :
			try :
				if trace :
						EKOX(e)
				ip = re.search("\((.*)\)", e).groups()[0]
				if trace :
						EKOX(ip)
				url = "http://" + ip + "/identify"
				headers = {'Accept': 'application/json'}
				r = requests.get(url, headers=headers, timeout=10)
				if trace :
						EKOX(r)
				j = r.json()
				if trace :
						EKOX(j)
				name = j["identity"]
				if trace :
						EKON(ip, name)
				self.devices[name] = ip
				if trace :
						EKOX(len(self.devices))
			except Exception as ex :
				if trace :
						EKOX(ex)
				pass
		EKO()
		EKOX(self.devices)

	def data(self) :
		"""
		lit le capteur salon
		"""
		# maison
		headers = {'Accept': 'application/json'}
		try :
			url = "http://" + self.devices["salon"] + "/temperature"
			#EKO()
			r = requests.get(url, headers=headers)
			j = r.json()
			j['DS18B20_salon'] = j['DS18B20']
		except :
			j = {
				'DS18B20_salon' : { 'value' : -999},
				'DS18B20' : { 'value' : -999},
				'DHT' : { 'temperature' : -999, 'hygrometry' : -999 },
				'MQ2' : { 'gaz' : -999 },
				'millis' : 0
				}
		# chaudiere
		headers = {'Accept': 'application/json'}
		try :
			url = "http://" + self.devices["chaudiere"] + "/temperature"
			r = requests.get(url, headers=headers)
			j1 = r.json()
			j['DS18B20'] = j1['DS18B20'] 
		except :
			j['DS18B20'] =	{ 'value' : -999}

		j['date'] = datetime.datetime.now().isoformat()
		EKOX(j['date'])
		if datetime.datetime.now() > get_t1() + datetime.timedelta(minutes = 10) or self.obs is None :
			#EKO()
			update_t1()
			self.obs = self.meteo.get_observation(48.216671,-1.75) # gps de la meziere
			self.suliac = self.meteo.get_observation(48.568886594144104, -1.975317996277762)

			dd = lambda f  : abs(datetime.datetime.fromtimestamp(f['dt']).hour - 12)
			
			# select forecast for st suliac in +2 days closest to 12am
			self.suliac_forecast = self.meteo.get_forecast(48.568886594144104, -1.975317996277762).forecast

			ff = [ f for f in self.suliac_forecast if (datetime.datetime.fromtimestamp(f['dt']) - get_t1()).days == 2 ]
			ff = sorted(ff, key = dd)
			self.suliac_forecast = ff[0]
			#EKOX(self.forecast)

			self.obs_forecast = self.meteo.get_forecast(48.216671,-1.75).forecast # gps de la meziere
			ff = [ f for f in self.obs_forecast if (datetime.datetime.fromtimestamp(f['dt']) - get_t1()).days == 2 ]
			ff = sorted(ff, key = dd)
			self.obs_forecast = ff[0]

		j['tempext'] = self.obs.temperature		   

		# speed m/s
		j['force_vent'] = self.obs.wind_speed
		#EKOX(self.obs.wind_speed)
		j['direction_vent'] = self.obs.wind_direction

		j['obs_forecast_force_vent'] = self.obs_forecast['wind']['speed']		 
		j['obs_forecast_direction_vent'] = self.obs_forecast['wind']['direction']

		#j['obs_forecast_date'] = datetime.datetime.fromtimestamp(self.obs_forecast['dt'])
		j['obs_forecast_date_iso'] = datetime.datetime.fromtimestamp(self.obs_forecast['dt']).isoformat()
		
		#EKOX( self.suliac.wind_speed)
		j['suliac_force_vent'] = self.suliac.wind_speed
		j['suliac_direction_vent'] = self.suliac.wind_direction
		j['suliac_forecast_force_vent'] = self.suliac_forecast['wind']['speed']		   
		j['suliac_forecast_direction_vent'] = self.suliac_forecast['wind']['direction']

		#j['suliac_forecast_date'] = datetime.datetime.fromtimestamp(self.suliac_forecast['dt'])
		j['suliac_forecast_date_iso'] = datetime.datetime.fromtimestamp(self.suliac_forecast['dt']).isoformat()
		
		j['tempchaudiere'] =  j['DS18B20']
		
		return j

	def save(self, folder = "/tmp") :
		saving_to = self.data_filename()
		EKOX(saving_to)
		with open(saving_to, "wb") as fd :
			pickle.dump(self.buffer, fd, protocol=pickle.HIGHEST_PROTOCOL)
		if not self.args.debug :
			with lzma.open(saving_to + ".xz", "wb") as f:
				pickle.dump(self.buffer, f, protocol=pickle.HIGHEST_PROTOCOL)

				
	def index(self) :
			return tasks_list.index(self)
			
	def name(self) :
		return "Task_%d" % self.interval_sec

	def info(self):
		ks = self.devices.keys()
		EKOX(ks)
		return self.name() + "=>" + "; ".join([ k + " : " + self.devices[k] for k in ks])

	def run(self):
		""" Method that runs forever """
		j = self.data()		   
		while True:
			if datetime.datetime.now() > self.save_time + datetime.timedelta(hours = SAVE_PERIOD_HOUR, minutes=SAVE_PERIOD_MINUTES)  :
				#self.save()
				self.save(DATA_DIR)
				self.save_time = datetime.datetime.now()
			if datetime.datetime.now() > self.check_time + self.delta_check :
				self.discover_nodemcu();
				self.check_time = datetime.datetime.now()
			sleep(self.interval_sec)
			try :
				j = self.data()
				self.buffer.append(j)
				EKOX(len(self.buffer))
				if len(self.buffer) > self.max_length :
					self.buffer.pop(0)
					
			except Exception as e :
				EKOX(e)


intervals = [
		(1*3600, 1000), # 1 heure
		(24*3600, 1000), # 1 jour 
		(7*24*3600, 1000), # 1 semaine
		(30*24*3600, 10000), # 1 mois			 
		(30*24*3600*12000, 10000) # 1 an			 
]

class HelloWorld(object):
	def __init__(self, args):
		self.tasks = [ Task(e[0], args, e[1]) for e in intervals]
		if False :
				for t in self.tasks :
						t.read_plot()
		EKO()

	@cherrypy.expose
	def save(self) :
		for t in self.tasks :
			t.save(DATA_DIR)
		return "saved"
			
	@cherrypy.expose
	def index(self):
		""" main 
		"""
		with open('./sensor.html', 'r') as file:
			data = file.read()
			data = data.replace("INFO", self.info())
			return data

	@cherrypy.expose
	def info(self) :
		def read(gi) :
			i = os.environ[gi] if gi in os.environ else ""
			return gi + "=" + i
		ss = read('GITINFO') + ", " + read("HOST") + ", " + read("DATE")
		ss += "<br>"
		ss += ", ".join([ "buffer %d, " %len(t.buffer) for t in self.tasks])
		ss += "<br>"
		ss += " \n ".join([ t.info() for t in self.tasks])		
		return ss

	@cherrypy.expose
	def identify(self) :
		d = { "identity" : "server_temperature" }
		return json.dumps(d)
			
	@cherrypy.expose
	def data(self) :
		d = self.tasks[0].data()
		return json.dumps(d)		 

	@cherrypy.expose
	def log(self, data=None) :
		#EKO()
		p = urlparse(data);
		rp = os.path.relpath(p.path, start = "/")
		print(rp)
		
	@cherrypy.expose
	def read(self, s=0) :
		s = int(s)
		EKOX(s)
		j = self.tasks[s]
		#EKOX(len(j.buffer))
		d = { 'buffer' : j.buffer, 'interval' : j.interval_sec }
		#EKOX(len(j.buffer))
		#EKOX(j.buffer)
		#cherrypy.response.headers["Access-Control-Allow-Origin"] = '*'
		#cherrypy.response.headers['Content-Type'] = 'application/json'
		#EKOX(d)
		return json.dumps(d) 


parser = argparse.ArgumentParser()
parser.add_argument("--debug", action="store_true")
args = parser.parse_args()


def main() :
	PATH = os.path.abspath(os.path.dirname(__file__))
	EKOX(PATH)
	class Root(object): pass
	cherrypy.config.update({'server.socket_host': '0.0.0.0'})

	port = 8093
	if "PORT" in os.environ :
		port = int(os.environ["PORT"])
	EKOX(port)
	config = {
		"/": {
			"tools.staticdir.on": True,
			"tools.staticdir.dir": PATH,
			'tools.staticdir.index': 'index.html',
			'tools.response_headers.headers': [('Content-Type', 'image/jpeg'), ('Access-Control-Allow-Origin', '*')],
		},
		'global' : {

#			 'server.ssl_module' : 'builtin',
#			 'server.ssl_certificate' : "cert.pem",
#			 'server.ssl_private_key' : "privkey.pem",
			
			'server.socket_host' : '0.0.0.0', #192.168.1.5', #'127.0.0.1',
			'server.socket_port' : port,
			'server.thread_pool' : 8,
			'log.screen': False,
			'log.error_file': './error.log',
			'log.access_file': './access.log'
		},
	}
	hello = HelloWorld(args)
	
	sleep(2)
	EKOT("running")
	cherrypy.quickstart(hello, "/", config=config) 
	hello.tasks[0].thread.join()

if __name__ == "__main__":
		main()
