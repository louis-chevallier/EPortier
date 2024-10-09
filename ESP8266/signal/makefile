SKETCH=signal

env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html

signal : env compile upload monitor

start :
	PORT=8085 IPADDRESS="\\\"176.188.228.22\\\"" make signal

startx :
	c++ ttt/segment.cpp -o segment
	./segment

FREQ?=160
MMU?=4816H
WSURL="ws://192.168.1.181/ws"
include ../makefile


