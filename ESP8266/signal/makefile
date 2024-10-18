
#sainsmart board

CORE=esp8266:esp8266:nodemcuv2
SKETCH=signal
CPORT=/dev/ttyACM0
env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html

signal : env compile upload monitor

start :
	make signal

startx :
	c++ ttt/segment.cpp -o segment
	./segment

FREQ?=160
MMU?=4816H
WSURL="ws://192.168.1.181/ws"
include ../makefile


