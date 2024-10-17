
$(warning $(CURDIR))

MIP?=192.168.1.95

CPORT?= /dev/ttyUSB0
WSURL?="ws://192.168.1.95/ws"
SKETCH?=garage
PORT ?= 9999
#160
FREQ?=80
MMU?=3232
#4816H
CORE?=esp8266:esp8266:nodemcuv2
DATE=$(shell date -r $(SKETCH).ino | sed s,' ','_',g)
$(warning date $(DATE))
# --build-property build.extra_flags=-DNOEKO=1
DEF=--build-property build.extra_flags=-DPORT=$(PORT) --build-property compiler.cpp.extra_flags="-DDATE=\"$(DATE)\" -DIPADDRESS=$(IPADDRESS) -DNOEKO=0"  $(OPTIONS)

SOURCES=$(wildcard $(CURDIR)/*.ino $(CURDIR)/*.html $(CURDIR)/*.js $(CURDIR)/makefile makefile)
#$(warning $(SOURCES))
# install RemoteDebug2

x : start

run :
	arduino-cli core update-index
	arduino-cli board list
	arduino-cli board listall mkr
	arduino-cli core search esp8266
	arduino-cli core search NodeMCU
	arduino-cli core search ESP32
	arduino-cli core install esp8266:esp8266
	arduino-cli core install esp32:esp32
	arduino-cli core install arduino:avr
#	arduino-cli core install asp8266
#	arduino-cli compile --fqbn arduino:samd:mkr1000 MyFirstSketch

start1 : compile upload


#- v
compile : $(CURDIR)/builds/$(SKETCH).ino.bin

%.bin : $(CURDIR)/$(SKETCH).ino $(SOURCES)
	cd ..; arduino-cli compile --fqbn $(CORE) $(SKETCH) $(DEF) --library $(CURDIR),$(CURDIR)/.. -j 0  --build-path $(CURDIR)/builds 

#	cd ..; arduino-cli compile --fqbn $(CORE):mmu=$(MMU),xtal=$(FREQ),exception=enabled $(SKETCH) $(DEF) --library $(CURDIR),$(CURDIR)/.. -j 0  --build-path $(CURDIR)/builds 

#--library $(CURDIR)



# --log -v
upload :
	-arduino-cli core list
	cd ..; arduino-cli upload --input-dir $(CURDIR)/builds -p $(CPORT) --fqbn $(CORE)  $(SKETCH)

ota :
	cd ..; arduino-cli upload --input-dir $(CURDIR)/builds -p $(CPORT) --fqbn $(CORE)  $(SKETCH)  --port $(MIP)
#--protocol network

monitor :
	plink $(CPORT) -serial -sercfg 115200,8,n,1,N

monitor_ws :
	cd ..; python ws.py --url $(WSURL)

lib :
#	arduino-cli lib search HC_SR04
#	arduino-cli lib install FTDebouncer
#	arduino-cli lib uninstall ESP8266TimerInterrupt
#	arduino-cli lib uninstall MCP4725
#	arduino-cli lib search MCP4725
	arduino-cli lib install "Adafruit MCP4725"
	arduino-cli lib install ESPAsyncWebServer
#	arduino-cli lib install TaskScheduler#
	arduino-cli lib install ESP8266TimerInterrupt
#	arduino-cli lib install SimpleFTPServer
	arduino-cli lib install ADS1X15
	-arduino-cli lib uninstall DueTimer


doc :
	pandoc readme.md --filter=mermaid-filter  -o readme.pdf 

up_file :
	esptool -cd nodemcu -cb 460800 -cp  $(CPORT) -ca 0x300000 -cf $(SPIFFS_IMG)
