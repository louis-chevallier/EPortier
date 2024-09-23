#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


String S;

long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()

/* pinout
   GPIO ==  numero dans le code
   Label == marque
   Label	GPIO	Input           Output                  Notes
   D0		GPIO16	no interrupt	no PWM  or I2C support	HIGH at boot, used to wake up from deep sleep                           HCB
   D1		GPIO05	OK	        OK	                often used as SCL (I2C)                                                 IN2 A
   D2		GPIO04	OK	        OK	                often used as SDA (I2C)                                                 IN1 A
   D3		GPIO00	pulled up	OK	                connected to FLASH button, boot fails if pulled LOW                     IN2 B in4
   D4		GPIO02	pulled up	OK	                HIGH at boot, connected to on-board LED, boot fails if pulled LOW       IN1 B in3
   D5		GPIO14	OK	        OK	                SPI (SCLK)                                                              ENB
   D6		GPIO12	OK	        OK	                SPI (MISO)                                                              HCA
   D7		GPIO13	OK	        OK	                SPI (MOSI)                                                              ENA
   D8		GPIO15	pulled to GND	OK	                SPI (CS), Boot fails if pulled HIGH                                     HCC
   RX		GPIO03	OK	        RX pin	                HIGH at boot                                                            Trigger                
   TX		GPIO01	TX pin	        OK	                HIGH at boot, debug output at boot, boot fails if pulled LOW            
   A0		ADC0	Analog Input	X	
*/

/**

RX/TX (1,3) sont connectée à l'usb, donc pas possible d'utiliser Serial pour le debug et pour un autre communication a la fois
solutions : 
  OTA pour changer le code à la volée
  utiliser serial swap pour diriger Serial sur les pins 15/13 ( = D8/D7)
 
 */

ESP8266WebServer server(80);


#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);


// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT) + "/main";


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO14, PIN=D5 on board
long PORTE=D5; // pour le relais de la porte

long PORTE_OUVERTE = A0;
long PORTE_FERMEE = D2;

String buf_serial;
bool swapped(false);

void swap() {
  delay(500);
  Serial.swap();
  delay(500);  
  //Serial.begin(115200); //Begin Serial at 115200 Baud
  swapped = !swapped;
  if (swapped) {
    begin(1200, SERIAL_7E1);
  } else {
    Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  }
  EKOX(swapped);
}

#define G(x) (S + "\"" + String(x) + "\"")

bool porte_ouverte() {
  // true : contact ouvert
  int a0 = analogRead(PORTE_OUVERTE);
  return a0 < 500;
}

bool porte_fermee() {
  // true : contact ouvert  
  int d = digitalRead(PORTE_FERMEE);
  return d;
}

void setup() {
  
  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);  

  Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  EKOT("starting");
  delay(500);

  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      EKOT("Waiting to connect...");
  }

  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr);  //Print the local IP
  

  page.replace("JSCODE", jscode);

  /*
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  */

  // Print the IP address
  //Serial.println(WiFi.localIP());
  server.on("/main96713", HTTP_GET, [](){
      EKO();
      start = count;
      EKOT("handle_index_main");
      //Print Hello at opening homepage
      String message("count =");
      message += String(count);
      server.send(200, "text/html", message.c_str());
      int v = ledv ? LOW : HIGH;
      ledv = !ledv;
      digitalWrite(PORTE, HIGH); 
      delay(2000);
      digitalWrite(PORTE, LOW); 
      EKOT("end");
    });

  server.on("/", HTTP_GET, [](){
      
      EKOT("index");
      int a0 = analogRead(PORTE_OUVERTE);
      EKOX(a0);
      EKOX(digitalRead(PORTE_FERMEE));
      String porte;
      porte += String(porte_ouverte() ? "ouverte" : "_");
      porte += " ";
      porte += String(porte_fermee() ? "fermee" : "_");
      
      String npage(page);
      npage.replace("PORT", String(PORT));
      npage.replace("IPADDRESS", String(IPADDRESS));
      npage.replace("WURL", WURL);
      npage.replace("PORTE", porte);
      
      server.send(505, "text/html", npage.c_str());
      EKOT("end");
    });

  server.on("/swap", HTTP_GET, []() {
    EKO();
    swap();
    String npage("{");
    npage += S + G("status") + " : " + G("ok");
    npage += " }";
    server.send(200, "text/json", npage.c_str());
  });
  
  server.on("/statut_porte", HTTP_GET, []() {
    String message = "POST form was:\n";
    //for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    //Serial.println(message);
    String statO(porte_ouverte() ? "ouverte" : "_");
    String statF(porte_fermee() ? "fermee" : "_");
    String json = "{";
    json += S + "\"porte_ouverte\" : \"" + statO + "\" , ";
    json += S + "\"porte_fermee\" : \"" + statF + "\" , ";
    json += S + "\"swapped\" : \"" + swapped + "\" , ";
    json += S + "\"buf_len\" : \"" + buf_serial.length() + "\"";
    json += "}";
    //EKOX(json);
    server.send(200, "text/json", json);
    
      
      
  });
  
  // Start the server
  server.begin(); //Start the server
  EKOT("setup");


  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);
  pinMode(PORTE, OUTPUT);
  digitalWrite(PORTE, LOW);  
  EKOT("Server listening");
}

long last = 0;
void loop() {
  server.handleClient();
  auto now = millis();
  if (swapped) {
    if (Serial.available()) {
      char c = Serial.read();
      buf_serial += String(c);
    }
  } else {
    if (buf_serial.length() > 0) {
      EKOX(buf_serial.length());
      String ss(">>>>");
      for (int i = 0; i < buf_serial.length(); i++) {
        //EKOX(int(buf_serial[i]));
        //EKOX(String(buf_serial[i]));
        if (buf_serial[i] < 254) {
          if (buf_serial[i] <= 13) {
            ss += "\n >>>>";
          } else
            ss += String(buf_serial[i]);
        } 
      }
      EKOT("start");
      EKOX(ss);
      EKOT("end");
      

      
      buf_serial = "";
    }
  }
  if (now > last + 1000) {
    last = now;
    EKO();
  }
  count += 1;
}
