
PORT=8080
IPADDRESS1=176.188.228.22
IPADDRESS="\\\"$(IPADDRESS1)\\\""

start :
#	OPTIONS= DEFS= PORT=$(PORT) IPADDRESS=$(IPADDRESS) make garage
	OPTIONS= DEFS= PORT=8080 IPADDRESS="\\\"176.188.228.22\\\"" make garage


env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html
	bin2c --ultra -H favicon.h favicon.ico

copy :
	cat code.js | sed s,WURL,http://$(IPADDRESS1):$(PORT),  > /mnt/NUC/www/EPortier/code.js

garage : copy env compile ota monitor_ws

check :
	wget -q http://192.168.1.181/ ; wc  index.html

FREQ?=160
MMU?=4816H
#WSURL="ws://192.168.1.95/ws"
MIP="192.168.1.115"
WSURL="ws://192.168.1.115/ws"
include ../makefile


