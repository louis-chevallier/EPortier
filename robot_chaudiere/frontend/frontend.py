
import cherrypy
from time import sleep
from threading import Thread
from utillc import *
import requests
from xml.etree import ElementTree


tree = ElementTree.fromstring(response.content)
class Task(object):
    def __init__(self, interval=1):
        self.interval = interval
        thread = threading.Thread(target=self.run, args=())
        thread.daemon = True                            # Daemonize thread
        thread.start()                                  # Start the execution

    def run(self):
        """ Method that runs forever """
        while True:
            # Do something

            url = "http://192.168.1.33/temperature"
            headers = {'Accept': 'application/json'}
            r = requests.get(url, headers=headers)
            EKOX(r.json())
            time.sleep(self.interval)


            
# create two new threads
task = Task()

def ff() :
    print(f'It took {end_time- start_time: 0.2f} second(s) to complete.')
    class HelloWorld(object):
        @cherrypy.expose
        def index(self):
            return "Hello World!"
        
    cherrypy.quickstart(HelloWorld())
