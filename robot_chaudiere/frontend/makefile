SHELL := /usr/bin/bash
export DATE:=$(shell date +%Y-%m-%d_%Hh%Mm%Ss)
export HOST=$(shell hostname)
SHELL=bash
export GITINFO=$(shell git log --pretty=format:"%h - %an, %ar : %s" -1)

REPO=https://github.com/louis-chevallier/EPortier.git
DEPLOY_DIR = /deploy
WOD?='$(shell fortune -s | sed -e 's/["]//g' | sed -e "s/[']//g")'


deploy :
	-git commit -a -m $(WOD)
	-git push
	cd $(DEPLOY_DIR); rm -fr EPortier; git clone $(REPO); cd EPortier/robot_chaudiere/frontend; make pem

startx :
	python pid.py

start : run

server  :
#	mypy --install-types --ignore-missing-imports  frontend.py
	echo bpython
	python frontend.py

#2>> trace.txt

pem : privkey.pem  privkey.pem

run :
	echo run
	date
	source ${HOME}/scripts/.bashrc; echo bspy; spy; echo bpyenv; pyenv; echo bmake; make server

nothing :
	echo done

install :
	cp sensor.html  /var/www/html/capteur_salon

%.pem :
# https://docs.cherrypy.dev/en/latest/deploy.html
	openssl genrsa -out privkey.pem 2048
	openssl req -new -x509 -days 365 -key privkey.pem -out cert.pem
