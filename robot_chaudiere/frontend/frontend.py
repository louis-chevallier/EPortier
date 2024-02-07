
import cherrypy, os
from time import sleep
from threading import Thread
from utillc import *
import requests
import time, json

max_length = 1000


class Task(object):
    def __init__(self, interval=1):
        self.interval = interval
        self.thread = Thread(target=self.run, args=())
        self.thread.daemon = True                            # Daemonize thread
        self.thread.start()                                  # Start the execution
        self.buffer = []
    def run(self):
        """ Method that runs forever """
        while True:
            sleep(self.interval)
            try :
                url = "http://192.168.1.33/temperature"
                headers = {'Accept': 'application/json'}
                r = requests.get(url, headers=headers)
                j = r.json()
                j['d'] = time.time
                #EKOX(j)
                self.buffer.append(r.json())
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
    def index__(self):
        return "Hello World!"

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
    cherrypy.config.update({
        "server.socket_port": 8088,
    })
    PATH = os.path.abspath(os.path.dirname(__file__))
    EKOX(PATH)
    class Root(object): pass
    cherrypy.config.update({'server.socket_host': '0.0.0.0'})
    config = {
        "/": {
            "tools.staticdir.on": True,
            "tools.staticdir.dir": PATH,
            'tools.staticdir.index': 'index.html',
            'tools.response_headers.headers': [('Content-Type', 'image/jpeg'), ('Access-Control-Allow-Origin', '*')],
        }
    }
    hello = HelloWorld()
    sleep(2)
    cherrypy.quickstart(hello, "/", config=config) 
    hello.tasks[0].thread.join()
