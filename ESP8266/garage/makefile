
start :
	OPTIONS= DEFS= PORT=8080 IPADDRESS="\\\"176.188.228.22\\\"" make garage

env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html
	bin2c --ultra -H favicon.h favicon.ico



garage : env compile upload  monitor

check :
	wget -q http://192.168.1.181/ ; wc  index.html

FREQ?=160
MMU?=4816H

include ../makefile


