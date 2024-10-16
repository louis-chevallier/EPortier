import urllib.request, json
import time
import datetime
import pickle
from utillc import *
import argparse
import matplotlib.pyplot as plt
import numpy as np
from datetime import timedelta
url = "http://192.168.1.115/data_linky"
d = {
    "values" : [],
    "dates" : {},
}
T = 1 # second
K = 3 #1000
parser = argparse.ArgumentParser(description='monitor linky')
parser.add_argument('--read', action='store_true')
parser.add_argument('--write', action='store_true')
args = parser.parse_args()
if args.read :
            with open("data.pickle", "rb") as f:
                d = pickle.load(f)
                dates = list(d["dates"].keys())
                EKOX(len(dates))
                dates = sorted(dates)
                d0 = dates[0]
                EKOX(d0)
                fi = d["dates"][d0]
                EKOX(fi)                
                v = np.asarray(d["values"][fi:])
                v = v[:,1]
                EKOX(v)
                n = len(v)
                EKOX(n)
                ndates = [ d0 + timedelta(seconds=s * T) for s in range(0,n)]
                plt.plot(ndates, v); plt.show()
if args.write :
    for i in range(1000000) :
        with urllib.request.urlopen(url) as p :
            data = json.load(p)
            #print(data["Iinst"])
            time.sleep(T)
            d["values"].append( (int(float(data["Iinst"]))*100, int(float(data["papp"]))))
        now = datetime.datetime.now()
        d["dates"][now] = i
        if i % K == 0 :
            EKOX(now)
            with open("data.pickle", "wb") as f:
                pickle.dump(d, f)
