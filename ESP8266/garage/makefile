
start :
	OPTIONS= DEFS= PORT=8080 IPADDRESS="\\\"176.188.228.22\\\"" make garage

env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html



garage : env compile upload check monitor

check :
	wget -q http://192.168.1.181/ ; wc  index.html

include ../makefile


