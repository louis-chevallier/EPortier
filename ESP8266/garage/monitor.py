import urllib.request, json
import time
import datetime
import pickle
from utillc import *
import argparse
import matplotlib.pyplot as plt
import numpy as np

url = "http://192.168.1.115/data_linky"
d = {}
parser = argparse.ArgumentParser(description='monitor linky')
parser.add_argument('--read', action='store_true')
args = parser.parse_args()
if args.read :
            with open("data.pickle", "rb") as f:
                d = pickle.load(f)
                dates = list(d.keys())
                EKOX(len(dates))
                dates = sorted(dates)
                #EKOX('\n'.join(list(map(str, dates[0:100]))))
                v = [ d[k][2] for k in  dates]
                plt.plot(v); plt.show()
else :
    for i in range(1000000) :
        with urllib.request.urlopen(url) as p :
            data = json.load(p)
            #print(data["Iinst"])
            time.sleep(1)
            now = datetime.datetime.now()
            d[now] = (data["Iinst"], data["papp"], data["pappm"])
        if i % 1000 == 0 :
            EKOX(now)
            with open("data.pickle", "wb") as f:
                pickle.dump(d, f)
