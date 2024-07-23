#include "ESPAsyncWebServer.h"

//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>    //Modification On The Air
#include <RemoteDebug.h>   //Debug via Wifi
#include <OneWire.h>
#include <SoftwareSerial.h>
String S;

long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()


OneWire  ds(8);  // on pin 10 (a 4.7K resistor is necessary)


//ESP8266WebServer server(80);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalClient(NULL);



// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
const char* WURL = "http://176.161.19.7:8080/main";
// deuxieme mcu 
//const char* WURL = "http://176.161.19.7:8081/main";

// chez pepito
//const char* ssid = "Bbox-09179E72"; //Enter Wi-Fi SSID
//const char* password =  "114564D1FA44C45A736FF6AE6D5E3C"; //Enter Wi-Fi Password
//const char* WURL = "http://176.161.19.7:8080/main";


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO15, PIN=D8 on board
long PINOUT=15;

//PINS - GPIO
#define RXD2 26
#define TXD2 27
#define LED 19


#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);

bool porte_ouverte() {
  Serial.println("statut porte");
  int a0 = analogRead(A0);
  return a0 < 500;
}

void send(const String &s) {
  if (globalClient != NULL) {
    globalClient->text(s);
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //EKOX(type);
  if(type == WS_EVT_CONNECT){
    EKOT("connect");
    EKOT("Websocket client connection received");
    globalClient = client;
 
  } else if(type == WS_EVT_DISCONNECT){
    EKOT("disconnect");
    EKOT("Websocket client connection finished");
    globalClient = NULL;
  } 
}

SoftwareSerial sserial(RXD2, TXD2);

void setup() {
  
  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  delay(10);

  // linky
  sserial.begin(1200); //, SERIAL_7E1, RXD2, TXD2);  //  7-bit Even parity 1 stop bit pour le Linky
  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  /////////////////////////////////

  ArduinoOTA
    .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";
        
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      });
  ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  
  ArduinoOTA.begin();


  ///////////////////////////////////////////////////////////
  
  page.replace("JSCODE", jscode);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  server.on("/main96713", HTTP_GET, [](AsyncWebServerRequest *request){
      start = count;
      Serial.print("handle_index_main");
      //Print Hello at opening homepage
      String message("count =");
      message += String(count);
      request->send(200, "text/html", message.c_str());
      //"Hello! This is an index page.");
      int v = ledv ? LOW : HIGH;
      ledv = !ledv;
      digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because 
      // it is acive low on the ESP-01)
      digitalWrite(PINOUT, HIGH); 
      delay(2000);
      digitalWrite(2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
      digitalWrite(PINOUT, LOW); 
      Serial.println("end");
    });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      
      Serial.println("index");
      int a0 = analogRead(A0);
      Serial.println("a0=" + String(a0));
      
      String porte(porte_ouverte() ? "ouverte" : "fermée");
      
      String npage(page);
      npage.replace("WURL", WURL);
      npage.replace("PORTE", porte);
      
      request->send(505, "text/html", npage.c_str());
      Serial.println("end");
    });


  
  server.on("/statut_porte", HTTP_GET, [](AsyncWebServerRequest *request) {
      String message = "POST form was:\n";
      //for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
      Serial.println(message);
      String stat(porte_ouverte() ? "ouverte" : "fermée");
      String json = "{ \"porte\" : \"" + stat + "\" }";
      Serial.println(json);
      request->send(200, "text/json", json);
  });
  
  server.begin(); //Start the server
  Serial.println("setup");
  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  
  pinMode(PINOUT, OUTPUT);
  digitalWrite(PINOUT, LOW);  
  Serial.println("Server listening");
}


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


void loop() {
  ArduinoOTA.handle();
  //server.handleClient(); //Handling of incoming client requests
  count += 1;
  //onewire();
}
