
init :
	usbreset "UNO R4 Minima DFU"
	sudo bash usb-rules.sh

start :
	OPTIONS= DEFS=  make r4

r4 : 
	OPTIONS="-DWS=0 -DDOSWAP=0" make compile upload monitor

CORE=arduino:renesas_uno:minima
CPORT=/dev/ttyACM0

include ../makefile


