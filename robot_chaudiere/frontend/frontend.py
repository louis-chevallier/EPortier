
import cherrypy, os
from time import sleep
from threading import Thread
from utillc import *
import requests
import time, json
import meteofrance_api
import datetime

max_length = 1000

class Task(object):
    def __init__(self, interval=1):
        self.meteo = meteofrance_api.MeteoFranceClient()
        self.interval = interval
        self.thread = Thread(target=self.run, args=())
        self.thread.daemon = True                            # Daemonize thread
        self.thread.start()                                  # Start the execution
        self.buffer = []
        self.obs = None
        self.t1 = datetime.datetime.now()

    def data(self) :
        url = "http://192.168.1.74/temperature"
        headers = {'Accept': 'application/json'}
        try :
            r = requests.get(url, headers=headers)
            j = r.json()
        except :
            j = {
                'DHT' : { 'temperature' : -999, 'hygrometry' : -999 },
                'MQ2' : { 'gaz' : -999 },
                'millis' : 0
                }
                
                
            #j['d'] = time.time


        if datetime.datetime.now() > self.t1 + datetime.timedelta(minutes = 10) or self.obs is None :
            EKO()
            self.t1 = datetime.datetime.now()
            self.obs = self.meteo.get_observation(48.216671,-1.75) # gps de la meziere
        j['tempext'] = self.obs.temperature
        
        j['tempchaudiere'] = 100.
        
        EKOX(j)
        return j
        
    def run(self):
        """ Method that runs forever """
        while True:
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
    def read(self, s=0) :
        s = int(s)
        EKOX(s)
        j = self.tasks[s]
        EKOX(len(j.buffer))
        d = { 'buffer' : j.buffer, 'interval' : j.interval }
        EKOX(len(j.buffer))
        EKOX(j.buffer)
        #cherrypy.response.headers["Access-Control-Allow-Origin"] = '*'
        #cherrypy.response.headers['Content-Type'] = 'application/json'
        return json.dumps(d) 
            
if __name__ == "__main__":
    PATH = os.path.abspath(os.path.dirname(__file__))
    EKOX(PATH)
    class Root(object): pass
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})

    port = 8093
    
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
    cherrypy.quickstart(hello, "/", config=config) 
    hello.tasks[0].thread.join()
