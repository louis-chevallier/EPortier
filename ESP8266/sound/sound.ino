//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

// à include avant asyncweb, sinon, ca crash
#include <FS.h>
#include "LittleFS.h"

#include "ESPAsyncWebServer.h"
#include "microTuple.h"
#include "ESP8266TimerInterrupt.h"
#include "websocket.h"

#include "util.h"
#include "tasks.h"

 
// chez nous
//const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* ssid = "Tenda-2.4G-ext"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT);


long count = 0;
int ledv = 1>2;
long start = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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

void setup() {

  eko_printer = new EKOPrinter1();
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
  jscode.replace("PORT", String(PORT));
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
  
}

long last = 0;
void loop() {
  auto now = millis();
  delay(4);    
  if (now > last + 1000) {
    last = now;
  }
  count += 1;
}
