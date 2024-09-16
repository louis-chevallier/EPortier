
SKETCH=test


env :
	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html


start :
	PORT=8080 IPADDRESS="\\\"176.188.228.22\\\"" make xx

xx : env compile upload

include ../makefile


