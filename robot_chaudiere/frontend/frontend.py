
import cherrypy, os, pickle
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

utillc.default_opt["with_date"] = 1

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
	
class Task(object):
	def __init__(self, duration_sec, max_length=1000):
		"""
		duration_sec : l'etendue temporelle couverte par cette tache
		max_length : le nombre d'échantillons sur cette période
		interval_sec : seconds
		"""
		self.max_length = max_length
		self.meteo = meteofrance_api.MeteoFranceClient()
		self.interval_sec = max(duration_sec / max_length, 30) # espace minimal entre 2 mesures : 30 sec
		EKON(duration_sec, max_length, self.interval_sec)
		self.buffer = []
		try :
			with open(os.path.join(DATA_DIR, "frontend_buffer_%05d.pickle" % self.interval_sec), "rb") as fd :
				self.buffer = pickle.load(fd)
		except Exception as e:
			EKOX(e)
		
		self.obs = None
		self.save_time = self.t2 = datetime.datetime.now() - 2 * datetime.timedelta(hours = 24) 
		self.check_time = self.t2 = datetime.datetime.now()


		self.devices = {}
		
		self.thread = Thread(target=self.run, args=())
		self.thread.daemon = True							 # Daemonize thread
		self.thread.start()									 # Start the execution
						 
	def discover_nodemcu(self) :
		batcmd="nmap -sL 192.168.1.*"
		result = subprocess.check_output(batcmd, shell=True, text=True)
		result = result.split("\n")
		#EKOX(result)

		#if 'salon' in self.devices :			 return
		
		for e in result :
			try :			 
				ip = re.search("\((.*)\)", e).groups()[0]
				#EKOX(ip)
				url = "http://" + ip + "/identify"
				headers = {'Accept': 'application/json'}
				r = requests.get(url, headers=headers)
				j = r.json()
				#EKOX(j)
				name = j["identity"]
				EKOX(name)
				self.devices[name] = ip
			except Exception as ex :
				pass
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
		j['obs_forecast_date'] = self.obs_forecast['dt']
		

		
		#EKOX( self.suliac.wind_speed)
		j['suliac_force_vent'] = self.suliac.wind_speed
		j['suliac_direction_vent'] = self.suliac.wind_direction
		j['suliac_forecast_force_vent'] = self.suliac_forecast['wind']['speed']		   
		j['suliac_forecast_direction_vent'] = self.suliac_forecast['wind']['direction']
		j['suliac_forecast_date'] = self.suliac_forecast['dt']
		
		j['tempchaudiere'] =  j['DS18B20']
		
		return j

	def save(self, folder = "/tmp") :
		saving_to = os.path.join(folder, "frontend_buffer_%05d.pickle" % self.interval_sec)
		EKOX(saving_to)
		with open(saving_to, "wb") as fd :
			pickle.dump(self.buffer, fd, protocol=pickle.HIGHEST_PROTOCOL)
			
	
	def run(self):
		""" Method that runs forever """
		j = self.data()		   
		while True:
			if datetime.datetime.now() > self.save_time + datetime.timedelta(hours = SAVE_PERIOD_HOUR, minutes=SAVE_PERIOD_MINUTES)  :
				self.save()
				self.save(DATA_DIR)
				self.save_time = datetime.datetime.now()
			if datetime.datetime.now() > self.check_time + datetime.timedelta(minutes=30) :
				self.discover_nodemcu();
				self.check_time = datetime.datetime.now()
			sleep(self.interval_sec)
			try :
				j = self.data()
				self.buffer.append(j)
				if len(self.buffer) > self.max_length :
					self.buffer.pop(0)
					
			except Exception as e :
				EKOX(e)

class HelloWorld(object):
	def __init__(self):
		self.tasks = [
			Task(1*3600), # 1 heure
			Task(24*3600), # 1 jour 
			Task(7*24*3600), # 1 semaine
			Task(30*24*3600, 10000), # 1 mois			 
			Task(30*24*3600*12, 10000) # 1 an			 
		]

	@cherrypy.expose
	def save(self) :
		for t in self.tasks :
			t.save()
			
	@cherrypy.expose
	def index(self):
		""" main 
		"""
		with open('./sensor.html', 'r') as file:
			data = file.read()
			data = data.replace("INFO", self.info())
			return data

	def info(self) :
		def read(gi) :
			i = os.environ[gi] if gi in os.environ else ""
			return gi + "=" + i
		ss = read('GITINFO') + ", " + read("HOST") + ", " + read("DATE")
		ss += "<br>"
		ss += ", ".join([ "buffer %d, " %len(t.buffer) for t in self.tasks])
		return ss

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
			
if __name__ == "__main__":
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
	hello = HelloWorld()
	sleep(2)
	EKOT("running")
	cherrypy.quickstart(hello, "/", config=config) 
	hello.tasks[0].thread.join()
