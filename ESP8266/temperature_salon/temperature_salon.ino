#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <List.hpp>
#include <DHT.h>
#include <Hash.h>
#include <Arduino.h>
#include <microTuple.h>


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
int relay[2] = {-1, -1};

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



void set_relay(int value, int relay_id) {
  // value = 1 => relais fermé, le bruleur peut chauffer ( sauf seuil haut atteint)
  //D3 = v;
  EKOX(value);

  relay[relay_id] = value;
  int r = D3 ? relay_id == 0 : D4;

  digitalWrite(r, value);
} 

float getTemperature() {
  //EKO();
  sensors.requestTemperatures();
  //EKO(); 
  float temperatureC = sensors.getTempCByIndex(0);
  //EKOX(temperatureC);
  return temperatureC;
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

 
 String page(R""""(
<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <!-- 
    <meta http-equiv="refresh" content="4">
    --!>
    <title>Thermostat / Chaudière</title>
    <style>
    .bb {
        font-size: 80px;
    }
    </style>
    <script src="https://cdn.plot.ly/plotly-2.27.0.min.js" charset="utf-8"></script>
  </head>
  <body>
  Temperature : TEMPERATURE °C
   <div id="temperature"> _____ </div>
   <div id="temperatureDHT"> _____ </div>
   <div id="hygrometrieDHT"> _____ </div>
   <div id="gaz"> _____ </div>
   <div id="consigne"> _____ </div>
   <div id="relay"> _____ </div>
   <div id="plot_temperature" style="width:1000px;height:550px;"> _____ </div>
   <script src="https://cdn.plot.ly/plotly-2.27.0.min.js" charset="utf-8"></script>
  <script>
  function eko(x) {
    console.log(x)
  }
  var temps= [33, 44];
  var trace1 = {
    y: temps,
    type: 'scatter'
  }
  var data = [trace1];
  //Plotly.newPlot('plot_temperature', data);
  
  function read_temperature() {
    console.log("read temperature");
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "/temperature");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
      eko("received")
      if (xhr.readyState == 4 && xhr.status == 200) {
        response = xhr.response;
        /*
        temps.push(response.temperature);i aussi mais peut-être plus vers 18h30 je fais au mie
        if (temps.length > 24*60) { // mn in a day
          temps.shift();
        }
        let labels = [];
        // on oublie ca, le buffer est géré dans l'arduino

        eko()
        temps = response.valeurs;
        */
        tempDHT = response.DHT.temperature;
        hygroDHT = response.DHT.hygrometry;
        gaz = response.MQ2.gaz;

        setd = function(l, v) {
           document.getElementById(l).innerHTML = l + "=" + v;
        }

        //setd("temperature", "" + response.temperature + "°C");
        setd("temperatureDHT", "" + tempDHT + "°C");
        setd("hygrometrieDHT", hygroDHT);
        setd("gaz", gaz); 
        setd("consigne", response.consigne);
        setd("relay", response.relay);
        let now = new Date();

        //console.log(temps); 
        let interval = xhr.response.interval;
        /*
        for (i in temps) {
          let dd = new Date(now.getTime() - i * interval);
          let ss = dd.toLocaleDateString('fr', { weekday:"long", hour:"numeric", minute:"numeric"});
          labels.unshift(dd);
        }

        var trace1 = {
          x : labels, 
          y : temps,
          type: 'scatter'
        };
        var data = [trace1];
        Plotly.newPlot('plot_temperature', data);
        */
      }
    }
    //console.log("received");
    setTimeout(read_temperature, 1000 * 60); // 1 mn
  }
  setTimeout(read_temperature, 1000);

  const d = new Date();
  let time = d.getTime();

  </script>
  
  </body>
</html>
)"""");

void handle_temperature() {
  //EKOT("handle temperature");
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

  json += S + "\"millis\" : " + String(millis()) + ", ";

  json += S + "\"consigne\" : " + String(consigne) + ", ";
  
  json += S + "\"relay bruleur\" : " + String(relay[0]) + ", ";

  json += S + "\"relay_circulateur\" : " + String(relay[1]) + ", ";

  json += S + "\"request_number\" : " + String(request_number);
  //EKO();

  json += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json.c_str());
  //EKO(); 
  request_number ++;
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
  String npage("ok");
  if (server.argName(0) == "value") {
    set_relay(server.arg(0).toInt(), server.arg(1).toInt());
  } else {
    npage = "fail";
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", npage.c_str());
  Serial.println("end");
};

void handle_consigne() {
  String npage("ok");
  if (server.argName(0) == "value") {
    set_consigne(server.arg(0).toFloat());
  } else {
    npage = "fail";
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", npage.c_str());
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
    String json = "{ identity : '";
    json += IDENTIFY;
    json += "'}";
    server.send(200, "text/json", json.c_str());    
  });

  server.on("/", handle_index); //Handle Index page
  server.on("/temperature", handle_temperature);
  server.on("/consigne", handle_consigne);
  server.on("/relay", handle_relay);
  
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

