//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

// à include avant asyncweb, sinon, ca crash
#include <FS.h>
#include "LittleFS.h"

#include "ESPAsyncWebServer.h"
#include "microTuple.h"
#include "ESP8266TimerInterrupt.h"
#include "ADS1X15.h"
#include "websocket.h"



#include "util.h"
#include "tasks.h"



// mcp
// https://github.com/RobTillaart/MCP_ADC
#include "MCP_ADC.h"


MCP3208 mcp28;
#define MCP3208_CS_PIN 25

uint32_t start, stop, 
         analog_read_time,
         analog_read_multiple_time;

const uint8_t num_channels = 8;
uint8_t channels_list[num_channels] = {
    0,1,2,3,4,5,6,7
};




 
// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
//const char* ssid = "Tenda-2.4G-ext"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(__PORT);

const int PIN_TOGGLE = 13;

ADS1114 ADS_1(0x49);

long count = 0;
int ledv = 1>2;
//long start = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const int analogInPin = A0; 

struct EKOPrinter1 : EKOPrinter {
  virtual void println(const String &ss) {
    if (websocket::globalClient != NULL) {
      websocket::globalClient->text(ss);
    } else {
      //EKOPrinter::println(ss);
    }
  }
};

#include "favicon.h"
const unsigned char *favicon = bin2c_favicon_ico;
int favicon_length = sizeof(bin2c_favicon_ico) / sizeof(char);
#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);

struct MyWSHandler : websocket::WSHandler{
};


/*
//    typedef std::function<void(const T&)> OnRemove;

LinkedList<IF> tasks(onremove);

*/
void listAllFilesInDir(String dir_path)
{
	Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
                  // print file names
                  EKOT(dir_path + dir.fileName());
		}
		if (dir.isDirectory()) {
			// print directory names
                  EKOT(dir_path + dir.fileName() + "/");
                  // recursive file listing inside new directory
                  listAllFilesInDir(dir_path + dir.fileName() + "/");
		}
	}
}


void create_file(const String &fn, const String &data, const String &mode = "w") {
  File file = LittleFS.open(fn, "w");
  assert(file != 0);
  file.print(data);
  file.close();
}

void create_file(const String &fn, const char unsigned *data, int length, const String &mode = "w") {
  File file = LittleFS.open(fn, "w");
  assert(file != 0);
  file.write(data, length);
  file.close();
}


File file(NULL);
long starting_ms = millis();
long bufs = 0;
long last = 0;
long l = 0;
const long K = 1024;
const int BUF_SIZE= 4 * K;
typedef int T;
T buf[BUF_SIZE];

void setup() {

  //eko_printer = new EKOPrinter1();
  Serial.begin(115200);
  EKOT("starting");
  delay(6000);

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
  if(!LittleFS.begin()){
    EKOT("An Error has occurred while mounting SPIFFS");
  } else {
    EKOT("SPIFFS ok");

    create_file("favicon.ico", favicon, favicon_length);
    create_file("code.js", jscode);
    create_file("page.html", page);
    
    listAllFilesInDir("/");
  }
  
  jscode.replace("RANDOM", String(random(255)));
  jscode.replace("PORT", String(__PORT));
  jscode.replace("IPADDRESS", String(IPADDRESS));
  jscode.replace("WURL", WURL);
  
  ws.onEvent(websocket::onWsEvent);
  server.addHandler(&ws);
  EKO();

  server.on("/code.js", HTTP_GET, [](AsyncWebServerRequest *request){
      String npage(jscode);
      EKO();
      EKOX(npage.length());
      request->send(200, "text/javascript", npage.c_str());
      EKOT("end");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/favicon.ico", "image/x-icon");
      EKOT("end");
  });
    
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      
      EKOT("index");
      String npage(page);
      npage.replace("DATE", String(DATE));
      npage.replace("WURL", WURL);
      request->send(200, "text/html", npage.c_str());
      EKOT("end");
    });



  
  // Start the server
  server.begin(); //Start the server
  delay(1000);
  EKOT("Server listening");
  delay(1000);
  EKO();

  file = LittleFS.open("/sound.bin", "w");
  assert(file != 0);

  for (int b = 0; b < 4; b++) {
    for(int i=0; i < BUF_SIZE ; i++) {
      int sensorValue = analogRead(analogInPin);
      buf[i] = sensorValue;
    }
    file.write((unsigned char*)buf, l * sizeof(T));
    EKO();
  }
  file.close();
  EKO();

  pinMode(PIN_TOGGLE, OUTPUT);  


  setup_MPC();
  
}


void loop() {

  // genere un creneau à 1000Hz / 2 sur la broche D7 ( 13)
  auto now = micros();
  delayMicroseconds(4);    
  if (now > last + 1000) {
    last = now;
    digitalWrite(PIN_TOGGLE, !digitalRead(PIN_TOGGLE));
  }
}




void setup_MPC()
{

  Serial.println(__FILE__);
  Serial.print("MCP_ADC_LIB_VERSION: ");
  Serial.println(MCP_ADC_LIB_VERSION);

  SPI.begin();

  Serial.println();
  Serial.println("ADC\tCHAN\tMAXVALUE");
  Serial.print("mcp28\t");
  Serial.print(mcp28.channels());
  Serial.print("\t");
  Serial.println(mcp28.maxValue());

  Serial.println("\nTiming in micros().\n");
  delay(10);

  Serial.println("***************************************\n");
  for (int s = 1; s <= 16; s++)
  {
    Serial.println(s * 1000000UL);
    mcp28.setSPIspeed(s * 1000000UL);
    mcp28.begin(MCP3208_CS_PIN);
    test();
  }

  testChannelsRead();
  Serial.println("done...");
}



void test()
{
  uint32_t val = 0;

  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.read(channel);
  }
  stop = micros();
  analog_read_time = stop - start;
  
  Serial.print("mcp28.read()\t8x: \t");
  Serial.println(analog_read_time);
  delay(10);


  start = micros();
  int16_t readings[num_channels];
  
  mcp28.readMultiple(channels_list, num_channels, readings);
  stop = micros();
  analog_read_multiple_time = stop - start;

  Serial.print("mcp28.readMultiple()\t8x: \t");
  Serial.println(stop - start);

  Serial.print("read() time / readMultiple() time \t");
  Serial.println((1.0 * analog_read_time) / analog_read_multiple_time);
  delay(10);


  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.differentialRead(channel);
  }
  stop = micros();
  Serial.print("mcp28.differentialRead() 8x: \t");
  Serial.println(stop - start);
  delay(10);

  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.deltaRead(channel);
  }
  stop = micros();
  Serial.print("mcp28.deltaRead()\t8x: \t");
  Serial.println(stop - start);
  Serial.println();
  delay(10);

}


void testChannelsRead() {
  Serial.println("***************************************\n");

  mcp28.setSPIspeed(8000000);  //  8 MHz
  mcp28.begin(MCP3208_CS_PIN);
  Serial.println("8000000");

  for (uint8_t numChannelsToRead = 1; numChannelsToRead <= num_channels; numChannelsToRead++) {

    delay(10);

    //  read()
    start = micros();
    for (uint8_t i = 0; i < numChannelsToRead; i++) {
      mcp28.read(i);
    }
    stop = micros();
    analog_read_time = stop - start;

    Serial.print("mcp28.read()\t");
    Serial.print(numChannelsToRead);
    Serial.print(": \t");
    Serial.print(analog_read_time);
    Serial.println("us");

    //  readMultiple()
    uint8_t channels_list[numChannelsToRead];
    for (uint8_t i = 0; i < numChannelsToRead; i++) {
      channels_list[i] = i;
    }

    delay(10);

    int16_t readings[numChannelsToRead];
    start = micros();
    mcp28.readMultiple(channels_list, numChannelsToRead, readings);
    stop = micros();
    analog_read_multiple_time = stop - start;

    Serial.print("mcp28.readMultiple()\t");
    Serial.print(numChannelsToRead);
    Serial.print(": \t");
    Serial.print(analog_read_multiple_time);
    Serial.println("us");
    Serial.print("read() time / readMultiple() time \t");
    Serial.println((1.0 * analog_read_time) / analog_read_multiple_time, 2);  //  print as float
    
    Serial.println("\n");
    delay(10);


    








    
  }

}
