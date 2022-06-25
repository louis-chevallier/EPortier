# EPortier

## Sur le NUC

- installer apache2 sur NUC : apt install apache2
- mettre le script html  dans /var/www/html
## sur la box :
  - donner une @ statique a NUC ( bail statique)
  - rediriger les ports sur la box : WAN 8083 => LN <ip nuc>:80 

  - donner une @ statique a ESP8266( bail statique)
  - rediriger les ports sur la box : WAN 8082 => LAN <ip esp>:80
  
  IP NUC = 192.168.0.157
  IP ESP = 192.168.9.156
  
  note : IP box = 78.207.134.29

## sur arduino
 
  le board = NodeMCU
  avec un chip = 8266
- sur linux installer l'ide arduino le plus récent
- configurer l'IDE ( chaine de compil)
  
-https://www.xtronical.com/basics/systems/esp8266mcu-et-al/setting-nodemcu-v3-esp-12e-esp8266-arduino-ide/

https://arduino.stackexchange.com/questions/36330/how-to-make-a-5-volt-relay-work-with-nodemcu

https://randomnerdtutori


## manip
  
sudo usermod -a -G dialout louis


sudo chown louis /dev/ttyUSB0
si c'est le port ouvert par arduino


===> point d'entree : http://176.161.19.7:8080/main"

c'est redirigé vers le nodemcu qui est à http://192.168.1.95/";
