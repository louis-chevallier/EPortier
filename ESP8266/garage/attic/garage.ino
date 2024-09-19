#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#endif

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


#endif

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

long PORTE_OUVERTE = A0
long PORTE_FERMEE = D4

//PINS - GPIO
#define RXD2 12
#define TXD2 14
#define LED 19


String buf_serial;
bool swapped(false);

void swap() {
  delay(500);
  Serial.swap();
  delay(500);  
  //Serial.begin(115200); //Begin Serial at 115200 Baud
  swapped = !swapped;
  EKOX(swapped);
}

#define G(x) (S + "\"" + String(x) + "\"")

bool porte_ouverte() {
  //EKOT("statut porte");
  int a0 = analogRead(PORTE_OUVERTE);
  return a0 < 500;
}

bool porte_fermee() {
  int d = digitalRead(PORTE_FERMEE);
  return d;
}

void setup() {
  
  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);  

  Serial.begin(115200); //Begin Serial at 115200 Baud
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
      //digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because 
      // it is acive low on the ESP-01)
      digitalWrite(PORTE, HIGH); 
      delay(2000);
      //digitalWrite(2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
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
    json += S + "\"porte_ouverte\" : \"" + statO + "\",";
    json += S + "\"porte_fermee\" : \"" + statF + "\"";
    json += "}";
    //Serial.println(json);
    server.send(200, "text/json", json);
    
      if (swapped) {
        EKOT("dragunai");
        String c =  Serial.readString();
        EKOT(c);
      }
      
      
  });
  
  // Start the server
  server.begin(); //Start the server
  EKOT("setup");
#endif

  // INPUT ANALOG
  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE, OUTPUT);
  digitalWrite(PORTE, LOW);  
  EKOT("Server listening");
}


#ifdef ONEWIRE

void onewire(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}

#endif


void loop() {
#ifdef OTA
  ArduinoOTA.handle();
#endif
  //server.handleClient(); //Handling of incoming client requests
  count += 1;
  //onewire();
}
