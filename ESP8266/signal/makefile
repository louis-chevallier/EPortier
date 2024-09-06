SKETCH=signal

env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html

signal : env compile upload


start :
	PORT=8085 IPADDRESS="\\\"176.188.228.22\\\"" make signal

startx :
	c++ ttt/segment.cpp -o segment
	./segment

include ../makefile


