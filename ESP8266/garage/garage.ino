//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

// à include avant asyncweb, sinon, ca crash
#include <FS.h>
#include "LittleFS.h"

#include "ESPAsyncWebServer.h"
#include "microTuple.h"
#include "ESP8266TimerInterrupt.h"


#include "util.h"
#include "tasks.h"

typedef std::function<void(void)> MyFunc;

typedef MicroTuple<int, MyFunc> IF_1;
typedef MicroTuple<int, float> IF_2;

void fff() {
}


IF_2 ddd(1, 3.2);
IF_1 ddd1(1, fff);

 
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


#define G(x) (S + "\"" + String(x) + "\"")
#define P(f,v) G(f) + " : " + G(v)
#define Acc(x) S + "{ " + x + " }"


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


//ESP8266WebServer server(80);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalClient(NULL);;

struct EKOPrinter1 : EKOPrinter {
  virtual void println(const String &ss) {
    if (globalClient != NULL) {
      globalClient->text(ss);
    } else {
      //EKOPrinter::println(ss);
    }
  }
};




#define UPLOAD_FILE 1
#if UPLOAD_FILE == 1

#include "favicon.h"
const unsigned char *favicon = bin2c_favicon_ico;
int favicon_length = sizeof(bin2c_favicon_ico) / sizeof(char);


/* pour héberger les pages html et js
 * pourait plutôt être mise sur des fichier du file system de l'ESP8266 ( mais demande un méchanisme de upload)
 * autre solution : mettre ces pages sur autre serveur
 */

#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);

#endif

void swap() {
  //delay(500);
  noInterrupts();
  Serial.swap();
  interrupts();
  //delay(500);  
  //Serial.begin(115200); //Begin Serial at 115200 Baud
  swapped = !swapped;
  /*
  if (swapped) {
    Serial.begin(1200, SERIAL_7E1);
  } else {
    Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  }
  */
}

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

MicroTuple<String, String> split(const String &mess, const String &sep = "?") {
  auto index = mess.indexOf(sep);
  return MicroTuple<String, String>(mess.substring(0, index), mess.substring(index+1));
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
  } else if(type == WS_EVT_DATA){
    //EKOX(len);
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      //Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      //EKOX(msg.c_str());
      auto t = split(msg);
      //EKOX(t.get<0>());
      //EKOX(t.get<1>());
      

      /*
        if(info->opcode == WS_TEXT)
        client->text("I got your text message");
        else
        client->binary("I got your binary message");
      */

    } else {
      EKO();
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }
      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());
      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

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

  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);  

  Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
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
  
  jscode.replace("PORT", String(PORT));
  jscode.replace("IPADDRESS", String(IPADDRESS));
  jscode.replace("WURL", WURL);
  
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  EKO();

  server.on("/create_file", HTTP_GET, [](AsyncWebServerRequest *request){
    create_file("test.txt", "test");
    create_file("code.js", jscode);
    String npage(page);
    npage.replace("WURL", WURL);
    create_file("page.html", npage);
    

    String nnpage = String("{") + G("status") + " : " + G("ok") +  " }";
    request->send(200, "text/json", nnpage.c_str());    
  });
  server.on("/read_file", HTTP_GET, [](AsyncWebServerRequest *request){

    String dd;

    File file = LittleFS.open("test.txt", "r");
    assert(file != 0);    
    EKOX(file);
    String s;
    while (file.available()) {
      auto c = file.read();
      s += String((char) c);
    }
    dd = G("val") + " : " + G(s) + " ,";    
    EKOX(s);
    file.close();

    String npage = String("{");
    npage += G("status") + " : " + G("ok") +  ",";
    npage += dd;
    npage += G("foo") + " : " + G("ok");
    npage += "}";
    request->send(200, "text/json", npage.c_str()); 
  });    


  // Print the IP address
  //Serial.println(WiFi.localIP());
  server.on("/main96713", HTTP_GET, [](AsyncWebServerRequest *request){
    EKOX(long(request));
    start = count;
    EKOT("handle_index_main");
    //Print Hello at opening homepage
    /*
    String message("count =");
    message += String(count);
    */
    //request->send(200, "text/html", message.c_str());

    //String npage("{");
    //npage += S + G("status") + " : " + G("ok");
    //npage += " }";
    
    
    int v = ledv ? LOW : HIGH;
    ledv = !ledv;
    EKOT("high");    
    digitalWrite(PORTE, HIGH);


    
    tasks::apres(2 * tasks::SEC_MC, [](){
      EKOT("low");
      digitalWrite(PORTE, LOW);
    });
    auto json = Acc(P("status", "ok"));
    EKOX(json);
    request->send(200, "text/json", json);
    
    EKOT("end");
  });



  server.on("/code.js", HTTP_GET, [](AsyncWebServerRequest *request){
      String npage(jscode);
      //EKOT(npage);
      request->send(200, "text/javascript", npage.c_str());
      EKOT("end");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/favicon.ico", "image/x-icon");
      EKOT("end");
  });
    
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      
      EKOT("index");
      int a0 = analogRead(PORTE_OUVERTE);
      EKOX(a0);
      EKOX(digitalRead(PORTE_FERMEE));
      String porte;
      porte += String(porte_ouverte() ? "ouverte" : "_");
      porte += " ";
      porte += String(porte_fermee() ? "fermee" : "_");
      
      String npage(page);
      npage.replace("WURL", WURL);
      npage.replace("PORTE", porte);

      //EKOT(npage);
      
      request->send(200, "text/html", npage.c_str());
      EKOT("end");
    });

  server.on("/swap", HTTP_GET, [](AsyncWebServerRequest *request) {
    EKO();
    swap();
    EKOX(swapped);
    
    String npage("{");
    npage += S + G("status") + " : " + G("ok");
    npage += " }";
    request->send(200, "text/json", npage.c_str());
  });
  
  server.on("/statut_porte", HTTP_GET, [](AsyncWebServerRequest *request) {
    //EKOT("statut");
    String message = "POST form was:\n";
    //for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    //Serial.println(message);
    String statO(porte_ouverte() ? "ouverte" : "_");
    String statF(porte_fermee() ? "fermee" : "_");
    String json = Acc(P("porte_ouverte", statO) + ", " +
                      P("porte_fermee", statF) + ", " +               
                      P("swapped", swapped) + ", " +
                      P("buf_len", buf_serial.length()));
    
    request->send(200, "text/json", json);
    //EKOX(long(globalClient));
      
  });


  
  // Start the server
  server.begin(); //Start the server
  delay(1000);

  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);
  pinMode(PORTE, OUTPUT);
  digitalWrite(PORTE, LOW);  
  EKOT("Server listening");
  
  delay(1000);
  EKO();
  //tasks::test();      
  EKO();

  EKO();
  float s = 3;
  for (int ii = 0 ; ii < 1000 * 10 * 8; ii++) {
    s += sqrt(abs(s));
  }
  EKO();

  
}

long last = 0;
void loop() {

  //server.handleClient();

  auto now = millis();
  if (globalClient != NULL && globalClient->status() == WS_CONNECTED) {
    /*
      EKO();
        auto r = random(0,100); 
        if (r == 1) {
        String randomNumber = String(random(0,100));
        globalClient->text(randomNumber);
        }
      */
    }
    delay(4);    
  
  if (swapped) {
    if (Serial.available()) {
      char c = Serial.read();
      buf_serial += String(c);
      if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
        globalClient->text(String(c));
      }

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
    //EKO();
  }
  count += 1;
}
