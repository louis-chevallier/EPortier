SKETCH?=rxtx
$(warning $(CURDIR))
$(warning $(dirname $(CURDIR)))

start :
	OPTIONS= DEFS= PORT=8080 IPADDRESS="\\\"176.188.228.22\\\"" make rxtx

rxtx : compile upload  monitor


FREQ?=160
MMU?=4816H
#WSURL="ws://192.168.1.95/ws"
WSURL="ws://192.168.1.115/ws"
include ../makefile


