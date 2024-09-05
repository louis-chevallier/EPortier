
#define SERVER 1


#ifdef SERVER
#include "ESPAsyncWebServer.h"
//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#endif

//#define OTA 1

#ifdef OTA
#include <ArduinoOTA.h>    //Modification On The Air
#endif

//#include <RemoteDebug.h>   //Debug via Wifi

#ifdef ONEWIRE
#include <OneWire.h>
#endif

//#include <SoftwareSerial.h>
String S;

long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()

/* pinout

   Label	GPIO	Input           Output                  Notes
   D0	GPIO16	no interrupt	no PWM  or I2C support	HIGH at boot, used to wake up from deep sleep                           HCB
   D1	GPIO05	OK	        OK	                often used as SCL (I2C)                                                 IN2 A
   D2	GPIO04	OK	        OK	                often used as SDA (I2C)                                                 IN1 A
   D3	GPIO00	pulled up	OK	                connected to FLASH button, boot fails if pulled LOW                     IN2 B in4
   D4	GPIO02	pulled up	OK	                HIGH at boot, connected to on-board LED, boot fails if pulled LOW       IN1 B in3
   D5	GPIO14	OK	        OK	                SPI (SCLK)                                                              ENB
   D6	GPIO12	OK	        OK	                SPI (MISO)                                                              HCA
   D7	GPIO13	OK	        OK	                SPI (MOSI)                                                              ENA
   D8	GPIO15	pulled to GND	OK	                SPI (CS), Boot fails if pulled HIGH                                     HCC
   RX	GPIO03	OK	        RX pin	                HIGH at boot                                                            Trigger                
   TX	GPIO01	TX pin	        OK	                HIGH at boot, debug output at boot, boot fails if pulled LOW            
   A0	ADC0	Analog Input	X	
*/


#ifdef ONEWIRE
OneWire  ds(8);  // on pin 10 (a 4.7K resistor is necessary)
#endif

#ifdef SERVER 
//ESP8266WebServer server(80);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);


#endif

#ifdef SERVER
AsyncWebSocketClient * globalClient(NULL);
#endif

// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT) + "/main";
// deuxieme mcu 
//const char* WURL = "http://176.161.19.7:8081/main";


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO15, PIN=D8 on board
long PINOUT=15; // pour le relais de la porte

//PINS - GPIO
#define RXD2 26
#define TXD2 27
#define LED 19


bool porte_ouverte() {
  //EKOT("statut porte");
  int a0 = analogRead(A0);
  return a0 < 500;
}

void send(const String &s) {

#ifdef SERVER
  if (globalClient != NULL) {
    globalClient->text(s);
  }
#endif
}

#ifdef SERVER
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  EKOX(type);
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
#endif

int use_linky = 0;

//SoftwareSerial sserial(RXD2, TXD2);


void setup() {
  
  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  EKOT("starting");
  delay(10);
  EKO();
  // linky
  if (use_linky) { 
    //sserial.begin(1200); //, SERIAL_7E1, RXD2, TXD2);  //  7-bit Even parity 1 stop bit pour le Linky
  }


#ifdef SERVER
  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      EKOT("Waiting to connect...");
  }

  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr);  //Print the local IP
  
  /////////////////////////////////
#endif
  
#ifdef OTA
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
      EKOT("\nEnd");
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
#endif

  ///////////////////////////////////////////////////////////


#ifdef SERVER
  page.replace("JSCODE", jscode);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  

  // Print the IP address
  //Serial.println(WiFi.localIP());
  server.on("/main96713", HTTP_GET, [](AsyncWebServerRequest *request){
      EKO();
      start = count;
      EKOT("handle_index_main");
      //Print Hello at opening homepage
      String message("count =");
      message += String(count);
      request->send(200, "text/html", message.c_str());
      int v = ledv ? LOW : HIGH;
      ledv = !ledv;
      digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because 
      // it is acive low on the ESP-01)
      digitalWrite(PINOUT, HIGH); 
      delay(2000);
      digitalWrite(2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
      digitalWrite(PINOUT, LOW); 
      EKOT("end");
    });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      
      EKOT("index");
      int a0 = analogRead(A0);
      EKOX(a0);
      
      String porte(porte_ouverte() ? "ouverte" : "fermée");
      
      String npage(page);
      npage.replace("PORT", String(PORT));
      npage.replace("IPADDRESS", String(IPADDRESS));
      npage.replace("WURL", WURL);
      npage.replace("PORTE", porte);
      
      request->send(505, "text/html", npage.c_str());
      EKOT("end");
    });


  
  server.on("/statut_porte", HTTP_GET, [](AsyncWebServerRequest *request) {
      String message = "POST form was:\n";
      //for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
      //Serial.println(message);
      String stat(porte_ouverte() ? "ouverte" : "fermée");
      String json = "{ \"porte\" : \"" + stat + "\" }";
      //Serial.println(json);
      request->send(200, "text/json", json);
  });
  
  // Start the server
  server.begin(); //Start the server
  EKOT("setup");
#endif

  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  
  pinMode(PINOUT, OUTPUT);
  digitalWrite(PINOUT, LOW);  
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
