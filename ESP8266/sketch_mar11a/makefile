
$(warning $(CURDIR))

# install RemoteDebug2

x : start

run :
	arduino-cli core update-index
	arduino-cli board list
	arduino-cli board listall mkr
	arduino-cli core search esp8266
	arduino-cli core search NodeMCU
	arduino-cli core install esp8266:esp8266
	arduino-cli core install asp8266
#	arduino-cli compile --fqbn arduino:samd:mkr1000 MyFirstSketch

start : compile upload

compile :

	bin2c --ultra -H code.h code.js
	bin2c --ultra -H page.h page.html
	cd ..; arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 sketch_mar11a --library $(CURDIR) --library $(CURDIR)

upload :
	-arduino-cli core list
	cd ..; arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 sketch_mar11a
	plink /dev/ttyUSB0 -serial -sercfg 115200,8,n,1,N

lib :
	arduino-cli lib search HC_SR04
#	arduino-cli lib install FTDebouncer


