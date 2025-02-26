#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <List.hpp>
#include <DHT.h>
#include <Hash.h>
#include <Arduino.h>
#include <microTuple.h>

#include <FS.h>
#include "LittleFS.h"


#include "util.h"
#include "UtilFS.h"

/*
long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + __LINE__ + ":[" + (millis()-seko) + "] " + String(x)); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + __LINE__ + ":[" + (millis()-seko) + "] " + #x + " = " + String(x)); seko=millis()
#define EKO() Serial.println(S + __FILE__+ ":" + __LINE__ + ":[" + (millis()-seko) + "]"); seko=millis()
*/
/********************************************/
// DHT11

// https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/

#define DHTTYPE    DHT11     // DHT 11
#define DHTPIN 5     // Digital pin connected to the DHT sensor : D1
typedef MicroTuple<float, float> FF;
DHT dht(DHTPIN, DHTTYPE);

long request_number = 0;
float aa = 3.2;

//int D3 = 3;

void DHTSetup() {
  dht.begin();
}

float nonan(float f) {
  return f != f ? -9999. : f;
}

FF readDHT() {
  float newT = nonan(dht.readTemperature());
  float newH = nonan(dht.readHumidity());

  auto t = FF(newT, newH);  	
  return t;
} 
/*********************************/
// MQ2
// https://mindstormengg.com/nodemcu-esp8266-lab-9-mq-2-gas-sensor/
int smokeA0 = A0;

// Your threshold value. You might need to change it.
int sensorThres = 600;

float temperature_bias = 0;
const String temperature_bias_fn = "/temperature_bias.txt";

void store_temperature_bias() {
  /*
  File file = LittleFS.open(temperature_bias_fn, "w");
  assert(file != 0);
  file.print(String(temperature_bias));
  file.close();
  */
}

float read_temperature_bias() {
  /*
  auto ss = read_file(temperature_bias_fn);
  return ss.toFloat();
  */
  return 0;
}


void MQ2Setup() {
  pinMode(smokeA0, INPUT);
}

int MQ2Read() {
  int analogSensor = analogRead(smokeA0);
  return analogSensor;
}

/*******************************/
// Temperature

OneWire  _ds(D5); // now on D5 // on pin D2 ( == GPIO4)  (a 4.7K resistor is necessary)
DallasTemperature sensors(&_ds);

int totalDurationMS = 24*60*60*1000;
//int totalDurationMS = 12*1000;
int delta = 60*1000; // intervals de mesure en ms
int period_automaton = 5*1000; // intervals de mesure en ms
//int delta = 3*1000; // intervals de mesure en ms
int nmbSamples = totalDurationMS / delta; 
List<float> temperatures;

auto last = millis();
auto last_automaton = millis();

float consigne = 25., delta_hysteresis = 1;
int relay[2] = {0, 0};

//https://randomnerdtutorials.com/esp8266-ds18b20-temperature-sensor-web-server-with-arduino-ide/

//https://projecthub.arduino.cc/m_karim02/arduino-and-mq2-gas-sensor-f3ae33

// pins number
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

void onewireSetup() {
  // Start the DS18B20 sensor
  sensors.begin();
}

void set_consigne(float v) {
  consigne = v;
} 

void set_temperature_bias(float v) {
  temperature_bias = v;
  store_temperature_bias(); 
} 

void set_relay(int value_to_set, int relay_id) {
  // value = 1 => relais fermé, le bruleur peut chauffer ( sauf seuil haut atteint)
  //D3 = v;
  EKOX(value_to_set);
  EKOX(relay_id);

  relay[relay_id] = value_to_set;
  int r = D3 ? relay_id == 0 : D4;

  digitalWrite(r, value_to_set);
  EKOT("done");
} 

float getTemperature() {
  //EKO();
  sensors.requestTemperatures();
  //EKO(); 
  float temperatureC = sensors.getTempCByIndex(0);
  //EKOX(temperatureC);
  return temperatureC + temperature_bias;
}

void collectTemperature(bool fake=1>2) {
  float t = fake ? 0. : getTemperature();
  temperatures.add(t); 
  if (temperatures.getSize() > nmbSamples) {
    temperatures.remove(0);
  } 
}

void onewireLoop() {
  sensors.requestTemperatures(); 
  float temperatureC = getTemperature();
  float temperatureF = sensors.getTempFByIndex(0);
  //Serial.print(temperatureC);
  //Serial.println("ºC");
  //Serial.print(temperatureF);
  //Serial.println("ºF");
  //delay(5000);
}


/********************************/
// webserver

ESP8266WebServer server(80);

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

#include "favicon.h"
String favicon = ((const char *)bin2c_favicon_ico);
int favicon_length = sizeof(bin2c_favicon_ico) / sizeof(char);

#include "page.h"
String page((const char*)bin2c_page_html);

#include "code.h"
String jscode((const char*)bin2c_code_js);

 

void handle_temperature() {
  EKOT("handle temperature");
  String json = "{";
  auto st = String(getTemperature());
  //json += String("{") + "\"temperature\" : " + st + "," ;
  /*
  //EKOX(temperatures.getSize());
  json += "\"valeurs\" : [ " ;
  for (int i = 0; i < temperatures.getSize(); i++) {
    if (i > 0) json += ",";
    json += String(temperatures[i]);
    //Serial.println(json);

  };
  json += "  ],";
  //EKO();
  */
  auto th = readDHT();
  json += S + "\"DHT\" : { \"temperature\" : " + th.get<0>() + ", \"hygrometry\" : " + th.get<1>() + "},";
  //EKO();
  json += S + "\"MQ2\" : { \"gaz\" : " + MQ2Read() + "},";

  json += S + "\"DS18B20\" : { \"value\" : " + String(st) + "},";

  json += S + "\"interval\" : " + String(delta) + ", ";

  json += S + "\"identity\" : \"" + IDENTIFY + "\", ";

  json += S + "\"millis\" : " + String(millis()) + ", ";

  json += S + "\"temperature_bias\" : " + String(temperature_bias) + ", ";
  //EKO();
  json += S + "\"consigne\" : " + String(consigne) + ", ";
  
  json += S + "\"relay_bruleur\" : " + String(relay[0]) + ", ";

  json += S + "\"relay_circulateur\" : " + String(relay[1]) + ", ";

  json += S + "\"request_number\" : " + String(request_number);
  //EKO();

  json += "}";
  //EKOX(json); 
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json.c_str());
  request_number ++;
  //EKO();
}

void handle_index() {
  Serial.println("index");
 
  String npage(page);
  npage.replace("TEMPERATURE", String(getTemperature()));
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", npage.c_str());
  Serial.println("end");
}

void handle_relay() {
  // method get
  String npage("{ \"status\" : \"ok\" }");
  EKOX(server.argName(0));
  EKOX(server.argName(1));
  if (server.argName(0) == "value") {
    set_relay(server.arg(0).toInt(), server.arg(1).toInt());
  } else {
    npage = "{ \"status\" : \"failed\" }";      
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", npage.c_str());
  Serial.println("end");
};

void handle_consigne() {
  String npage("{ \"status\" : \"ok\" }");  
  if (server.argName(0) == "value") {
    set_consigne(server.arg(0).toFloat());
  } else {
    npage = "{ \"status\" : \"failed\" }";      
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", npage.c_str());
  Serial.println("end");
}

void handle_temperature_bias() {
  String npage("{ \"status\" : \"ok\" }");  
  if (server.argName(0) == "value") {
    set_temperature_bias(server.arg(0).toFloat());
  } else {
    npage = "{ \"status\" : \"failed\" }";      
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", npage.c_str());
  Serial.println("end");
}

void webSetup() {
  
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  EKOX(WiFi.localIP().toString());  //Print the local IP
  
  // Start the server
  server.begin();
  EKOT("Server started");

  // Print the IP address
  server.on("/identify", [](){
    String npage(IDENTIFY);
    String json = "{ \"identity\" : \"";
    json += IDENTIFY;
    json += "\"}";
    server.send(200, "text/json", json.c_str());    
  });

  server.on("/", handle_index); //Handle Index page
  server.on("/temperature", handle_temperature);
  server.on("/set_consigne", handle_consigne);
  server.on("/set_relay", handle_relay);
  server.on("/set_temperature_bias", handle_temperature_bias);

  server.on("/favicon.ico", [](){
    EKOT("icon");
    server.send(200, "image/x-icon", favicon);
    EKOT("end");
  });

  server.on("/code.js", []() {
    EKOT("js");
    String npage(jscode);
    EKO();
    EKOX(npage.length());
    server.send(200, "text/javascript", npage);
    EKOT("end");
  });

  
  
  server.begin(); //Start the server
  EKOT("setup");
  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  
  EKOT("Server listening");
}

void webLoop() {
  server.handleClient(); //Handling of incoming client requests
}

/***********************/


void  setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  EKOX(aa);
  delay(10);
  webSetup();
  onewireSetup();
  EKOX(getTemperature());
  pinMode(D3, OUTPUT);  
  pinMode(D4, OUTPUT);
  //pinMode(D2, INPUT_PULLUP);  
  /*
  
  delay(5); 
  onewireLoop();
  //Serial.println(code);
  Serial.println("ready...");

  Serial.println(S + "nmsSamples " + nmbSamples);

  for (int i = 0; i < nmbSamples; i++) {
    collectTemperature(1>0); 
  } 
  */
  DHTSetup();
  EKOX(readDHT().get<0>());
  EKOX(readDHT().get<1>());
  MQ2Setup();
  EKOX(MQ2Read());

  EKOX(getTemperature());

  Serial.println("ok");
  Serial.println(D1);
  Serial.println(D2);
  Serial.println(D3);
  Serial.println(D4);
  Serial.println(D5);

  EKOX(WiFi.localIP().toString());
  
  set_relay(0, 0);
  set_relay(0, 1);

  //temperature_bias = read_temperature_bias();
  
}

int state = 0;
void automaton()  {
  //EKOX(state);
  //EKOX(millis());
  switch (state) {
    case 0 : state = 1;
    break;

    case 1 : if (getTemperature() <  consigne - delta_hysteresis) { state = 2; set_relay(1, 0); }
    break;

    case 2 : if (getTemperature() >  consigne + delta_hysteresis) { state = 1; set_relay(0, 0); }
    break;
 
  }

}


void loop() {
  webLoop();
  count += 1;
  auto ddn = millis();
  if (ddn - last > delta) {
    last = ddn;
    collectTemperature();
  }
  if (ddn - last_automaton > period_automaton) {
    last_automaton = ddn;
    automaton();
  }
  if (ddn < last) {
    last = 0;
  }
}

