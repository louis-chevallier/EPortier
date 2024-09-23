
$(warning $(CURDIR))

SKETCH?=garage
PORT ?= 9999
DEF= --build-property build.extra_flags=-DPORT=$(PORT) --build-property compiler.cpp.extra_flags=-DIPADDRESS=$(IPADDRESS)


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

start1 : compile upload




compile :
	cd ..; arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 $(SKETCH) $(DEF) --library $(CURDIR)

#--library $(CURDIR)

upload :
	-arduino-cli core list
	cd ..; arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2  $(SKETCH)
	plink /dev/ttyUSB0 -serial -sercfg 115200,8,n,1,N

lib :
#	arduino-cli lib search HC_SR04
#	arduino-cli lib install FTDebouncer
#	arduino-cli lib uninstall ESP8266TimerInterrupt
#	arduino-cli lib uninstall MCP4725
#	arduino-cli lib search MCP4725
	arduino-cli lib install "Adafruit MCP4725"
	arduino-cli lib install ESPAsyncWebServer


doc :
	pandoc readme.md --filter=mermaid-filter  -o readme.pdf 
