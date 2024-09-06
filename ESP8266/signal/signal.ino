
#include "ESPAsyncWebServer.h"
#include <Adafruit_MCP4725.h>
Adafruit_MCP4725 dac;

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


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);
AsyncWebSocketClient * globalClient(NULL);

long request_number(0);

// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT) + "/main";


long count = 0;
int ledv = 1>2;
long start = 0;

//PINS - GPIO
#define RXD2 26
#define TXD2 27
#define LED 19

#include "segment.hpp"

auto r1 = Ramp(1000, 0, 1);
auto signal1 = rev(r1);
auto signal = repeat(cat(r1, rev(r1)), 10000);

void send(const String &s) {
  if (globalClient != NULL) {
    globalClient->text(s);
  }
}

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

void handle_data(AsyncWebServerRequest *request) {
  String jx, jy;
  auto step_s = request->getParam("step")->value();
  EKOX(step_s);
  auto step = step_s.toFloat();
  EKOX(signal.duration_ms());
  EKOX(signal.data(10));
  for (int t_ms = 0; t_ms < signal.duration_ms(); t_ms += step) {
    if (t_ms > 0) jx += " ,"; 
    jx += String(float(t_ms) / 1000); // sec
    if (t_ms > 0) jy += " ,"; 
    jy += String(signal.data(t_ms));
    if ((t_ms / step) > 300) {
      break;
    }
  }

  /*
  for (int i = 0; i < 100; i++) {
    if (i > 0) jx += " ,"; 
    jx += String(i);
    if (i > 0) jy += " ,"; 
    jy += String(i*i);
  }
  */
  String json = S + "{ \"x\" : [" + jx + "], \"y\" : [" + jy + "]}" ;
  //EKOX(json);
  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json.c_str());
  //response->addHeader("Refresh", "3");
  //request->sendHeader("Access-Control-Allow-Origin", "*");
  //request->send(200, "application/json", json.c_str());
  request->send(response);
  
  request_number ++;
  EKOX(request_number);
}

void setup() {
  
  pinMode(A0,INPUT);

  dac.begin(0x60);
  
  Serial.begin(115200); //Begin Serial at 115200 Baud
  EKOT("starting");
  EKOX(signal.duration_ms());
  delay(10);
  EKO();

  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      EKOT("Waiting to connect...");
  }

  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr);  //Print the local IP


  page.replace("JSCODE", jscode);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){      
      EKOT("index");
      int a0 = analogRead(A0);
      EKOX(a0);
      String npage(page);
      npage.replace("PORT", String(PORT));
      npage.replace("IPADDRESS", String(IPADDRESS));
      npage.replace("WURL", WURL);
      request->send(505, "text/html", npage.c_str());
      EKOT("end");
    });

  server.on("/value", HTTP_GET, [](AsyncWebServerRequest *request){      
      int a0 = analogRead(A0);
      String json = S + "{ \"value\" : " + a0 +  "}" ;
      EKOX(json);
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json.c_str());
      request->send(response);      
      EKOX(a0);

    });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
      EKOT("data");
      handle_data(request);
    });

  
  // Start the server
  server.begin(); //Start the server
  EKOT("setup");

  //pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  //digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  dac.setVoltage(4095, true);
  EKOT("Server listening");
}

long sss(0);

long last = 0;
void loop() {
  count += 1;
  auto now = millis();

  if (now % 10 == 0) {
    /*
    if (sss++ < 100) {
      EKOX(now);
      EKOX(signal.data(now));
    }
    */
    //dac.setVoltage(signal.data(now) * 4095, true);
  }

  if (now > last + 1000) {
    last = now;
    int a0 = analogRead(A0);
    String json = S + "{ \"value\" : " + a0 +  "}" ;
    send(json);
  }
  
}
