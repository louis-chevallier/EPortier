
PORT=8080
IPADDRESS1?=192.168.1.181
IPADDRESS1?=176.188.228.22
IPADDRESS="\"$(IPADDRESS1)\""
OPTIONS?="-DWS=1 -DDOSWAP=1"

GITINFO="$(shell git log --pretty=format:"%h - %an, %ad : %s %n" -1)"
$(warning GITINFO=$(GITINFO))
MIP=$(IPADDRESS)

#MIP="192.168.1.115"
WSURL="ws://192.168.1.115/ws"

#CORE=esp8266:esp8266:nodemcuv2:mmu=$(MMU),xtal=$(FREQ),exception=enabled
CORE=esp8266:esp8266:nodemcuv2

start :
#	OPTIONS= DEFS= PORT=$(PORT) IPADDRESS=$(IPADDRESS) make garage
	OPTIONS= DEFS= PORT=8080 IPADDRESS="\\\"$(IPADDRESS1)\\\"" make garage1

env :
	echo $(GITINFO) > gitinfo.txt
	bin2c --ultra -H gitinfo.h gitinfo.txt
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H code_boot.h code_boot.js
	bin2c --ultra -H page.h page.html
	bin2c --ultra -H favicon.h favicon.ico

copy :
	cat code.js | sed s,WURL,http://$(IPADDRESS1):$(PORT),  > /mnt/NUC/www/EPortier/code.js

garage1 : 
	OPTIONS="-DWS=1 -DDOSWAP=0" make copy env compile ota monitor_ws
garage : 
	OPTIONS="-DWS=0 -DDOSWAP=0" make copy env compile upload monitor

check :
	wget -q http://192.168.1.181/ ; wc  index.html

#FREQ?=160
MMU?=4816H
#WSURL="ws://192.168.1.95/ws"


include ../makefile


