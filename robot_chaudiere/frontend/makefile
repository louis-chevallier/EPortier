SHELL := /usr/bin/bash

start  :
	python frontend.py

#2>> trace.txt

run :
	source ${HOME}/scripts/.bashrc; spy; python_env; make start

nothing :
	echo done

install :
	cp sensor.html  /var/www/html/capteur_salon
