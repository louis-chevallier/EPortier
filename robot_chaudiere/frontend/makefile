SHELL := /usr/bin/bash
export DATE:=$(shell date +%Y-%m-%d_%Hh%Mm%Ss)
export HOST=$(shell hostname)
SHELL=bash
export GITINFO=$(shell git log --pretty=format:"%h - %an, %ar : %s" -1)

start  :
	python frontend.py

#2>> trace.txt

run : privkey.pem  privkey.pem
	date
	source ${HOME}/scripts/.bashrc; spy; pyenv; make start

nothing :
	echo done

install :
	cp sensor.html  /var/www/html/capteur_salon

%.pem :
# https://docs.cherrypy.dev/en/latest/deploy.html
	openssl genrsa -out privkey.pem 2048
	openssl req -new -x509 -days 365 -key privkey.pem -out cert.pem
