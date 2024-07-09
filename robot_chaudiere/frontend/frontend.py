
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

utillc.default_opt["with_date"] = 1

max_length = 1000

DATA_DIR="/deploy/data"

class Task(object):
    def __init__(self, interval:int = 1):
        self.meteo = meteofrance_api.MeteoFranceClient()
        self.interval = interval
        self.buffer = []
        try :
            with open(os.path.join(DATA_DIR, "buffer_%05d.pickle" % self.interval), "rb") as fd :
                self.buffer = pickle.load(fd)
        except Exception as e:
            EKOX(e)
        
        self.obs = None
        self.save_time = self.t1 = datetime.datetime.now() - 2 * datetime.timedelta(hours = 24) 
    
        self.thread = Thread(target=self.run, args=())
        self.thread.daemon = True                            # Daemonize thread
        self.thread.start()                                  # Start the execution

    def data(self) :

        # maison
        url = "http://192.168.1.74/temperature"
        headers = {'Accept': 'application/json'}
        try :
            EKO()
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
        url = "http://192.168.1.33/temperature"
        headers = {'Accept': 'application/json'}
        try :
            r = requests.get(url, headers=headers)
            j1 = r.json()
            j['DS18B20'] = j1['DS18B20'] 
        except :
            j['DS18B20'] =  { 'value' : -999}

            
        if datetime.datetime.now() > self.t1 + datetime.timedelta(minutes = 10) or self.obs is None :
            EKO()
            self.t1 = datetime.datetime.now()
            self.obs = self.meteo.get_observation(48.216671,-1.75) # gps de la meziere
            self.suliac = self.meteo.get_observation(48.568886594144104, -1.975317996277762)

            # select forecast for st suliac in +2 days closest to 12am
            self.suliac_forecast = self.meteo.get_forecast(48.568886594144104, -1.975317996277762).forecast
            dd = lambda f  : abs(datetime.datetime.fromtimestamp(f['dt']).hour - 12)
            ff = [ f for f in self.suliac_forecast if (datetime.datetime.fromtimestamp(f['dt']) - self.t1).days == 2 ]
            ff = sorted(ff, key = dd)
            self.forecast = ff[0]
            EKOX(self.forecast)

        j['tempext'] = self.obs.temperature        

        # speed m/s
        j['force_vent'] = self.obs.wind_speed
        EKOX(self.obs.wind_speed)
        j['direction_vent'] = self.obs.wind_direction
        EKOX( self.suliac.wind_speed)
        j['suliac_force_vent'] = self.suliac.wind_speed
        
        j['suliac_direction_vent'] = self.suliac.wind_direction
        j['suliac_forecast_force_vent'] = self.forecast['wind']['speed']        
        j['suliac_forecast_direction_vent'] = self.forecast['wind']['direction']
        j['suliac_forecast_date'] = self.forecast['dt']
        
        j['tempchaudiere'] =  j['DS18B20']
        
        return j

    def save(self) :
        saving_to = "/tmp/buffer_%05d.pickle" % self.interval
        EKOX(saving_to)
        with open(saving_to, "wb") as fd :
            pickle.dump(self.buffer, fd, protocol=pickle.HIGHEST_PROTOCOL)
            
    
    def run(self):
        """ Method that runs forever """
        j = self.data()        
        while True:
            if datetime.datetime.now() > self.save_time + datetime.timedelta(hours = 24)  :
                self.save()
                self.save_time = datetime.datetime.now()
            sleep(self.interval)
            try :
                j = self.data()
                self.buffer.append(j)
                if len(self.buffer) > max_length :
                    self.buffer.pop(0)
                    
            except Exception as e :
                EKOX(e)

class HelloWorld(object):
    def __init__(self):
        self.tasks = [
            Task(1*3600/max_length),
            Task(24*3600/max_length),
            Task(7*24*3600/max_length)
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
        return read('GITINFO') + ", " + read("HOST") + ", " + read("DATE")

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
        #EKOX(s)
        j = self.tasks[s]
        #EKOX(len(j.buffer))
        d = { 'buffer' : j.buffer, 'interval' : j.interval }
        #EKOX(len(j.buffer))
        #EKOX(j.buffer)
        #cherrypy.response.headers["Access-Control-Allow-Origin"] = '*'
        #cherrypy.response.headers['Content-Type'] = 'application/json'
        return json.dumps(d) 
            
if __name__ == "__main__":
    PATH = os.path.abspath(os.path.dirname(__file__))
    EKOX(PATH)
    class Root(object): pass
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})

    port = 8093
    if "PORT" in os.environ :
        port = int(os.environ["PORT"])
    
    config = {
        "/": {
            "tools.staticdir.on": True,
            "tools.staticdir.dir": PATH,
            'tools.staticdir.index': 'index.html',
            'tools.response_headers.headers': [('Content-Type', 'image/jpeg'), ('Access-Control-Allow-Origin', '*')],
        },
        'global' : {
            'server.ssl_module' : 'builtin',
            'server.ssl_certificate' : "cert.pem",
            'server.ssl_private_key' : "privkey.pem",
            
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
