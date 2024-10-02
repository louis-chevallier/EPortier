#include <FS.h>
#include "LittleFS.h"
#include <SimpleFTPServer.h>
#include "util.h"
 
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial


// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT) + "/main";

void setup() {
  Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  EKOT("starting");
  delay(500);

  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.print(".");
  }

  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr);  //Print the local IP
  
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    EKOT("An Error has occurred while mounting SPIFFS");
  } else {
    EKOT("SPIFFS ok");
    ftpSrv.begin("esp8266","esp8266");
    EKOT("ftp server started");
  }
  
}

void loop(void) {
  ftpSrv.handleFTP();
  auto now = millis();
}
