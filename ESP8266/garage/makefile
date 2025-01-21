
# 8080 : port externe redirigé vers nodemcugarage par box
# 8086 : port externe redirigé vers nodemcu03 par box
PORT?=8080

#IPADDRESS1?=192.168.1.181
IPADDRESS1?=176.188.228.22

IPADDRESS="\"$(IPADDRESS1)\""
OPTIONS?="-DWS=1 -DDOSWAP=1"

GITINFO="$(shell git log --pretty=format:"%h - %an, %ad : %s %n" -1)"
$(warning GITINFO=$(GITINFO))
MIP=$(IPADDRESS)

#MIP="192.168.1.115"
WSURL?="ws://$(IPADDRESS1):$(PORT)/ws"

#WSURL="ws://192.168.1.6/ws"

#CORE=esp8266:esp8266:nodemcuv2:mmu=$(MMU),xtal=$(FREQ),exception=enabled
CORE=esp8266:esp8266:nodemcuv2


TESTING="\\\"testing\\\""
DEPLOY="\\\"deploy\\\""

start :
	make go

go :
#	OPTIONS= DEFS= PORT=$(PORT) IPADDRESS=$(IPADDRESS) make garage
	OPTIONS= DEFS= PORT=$(PORT) MODE=$(DEPLOY) IPADDRESS="\\\"$(IPADDRESS1)\\\"" make garage

test :
	OPTIONS= DEFS= PORT=8086 MODE=$(DEPLOY) IPADDRESS="\\\"$(IPADDRESS1)\\\"" make garage_test

install :
	pip install rel
env :
	echo $(GITINFO) > gitinfo.txt
	bin2c --ultra -H gitinfo.h gitinfo.txt
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H code_boot.h code_boot.js
	bin2c --ultra -H page.h page.html
	bin2c --ultra -H favicon.h favicon.ico

copy :
	cat code.js | sed s,WURL,http://$(IPADDRESS1):$(PORT),  > /mnt/NUC/www/EPortier/code.js

garage_test : 
	OPTIONS="-DWS=1 -DDOSWAP=0" make copy env compile upload monitor

garage : 
	OPTIONS="-DWS=1 -DDOSWAP=0" make copy env compile upload monitor

check :
	-rm checking/*
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/ )
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/code_boot.js )
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/code.js )
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/data_linky )
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/statut_porte)
	(cd checking; wget -q http://$(IPADDRESS1):$(PORT)/main96713)
	sleep 1
	diff refs/index.html checking/index.html
	diff refs/code.js checking/code.js
	diff refs/code_boot.js checking/code_boot.js

LAST := 10000
NUMBERS := $(shell seq 1 ${LAST})

check_loop :
	@$(foreach var,$(NUMBERS),make check;) 


#FREQ?=160
MMU?=4816H
#WSURL="ws://192.168.1.95/ws"


include ../makefile


