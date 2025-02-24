//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>

// à include avant asyncweb, sinon, ca crash
#include <FS.h>
#include "LittleFS.h"
#include "ESPAsyncWebServer.h"
#include "microTuple.h"
#include "ESP8266TimerInterrupt.h"
//#include <ArduinoOTA.h> 

//#include "NTPClient.h"
//#include "WiFiUdp.h"

#include "util.h"
#include "UtilFS.h"
#include "tasks.h"

#include <DoubleLinkedList.hpp>

enum {
  SWAP
};

struct Message {
  int c;
  Message(int i=0) : c(i) {}
};

DoubleLinkedList<Message> messages;

void push_message(const Message &m) {
  noInterrupts();
  messages.addAtIndex(messages.getSize(), (Message &)m);
  interrupts();
}

Message take_message() {
  noInterrupts();
  auto m = messages.get(0);
  messages.remove(0);
  interrupts();
  return m;
}

bool messages_available() {
  noInterrupts();
  auto b = messages.getSize() > 0;
  interrupts();
  return b;
}


/*
#include <Ethernet.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //physical mac address
char serverName[] = "http://192.168.1.40/heure"; // zoomkat's test web page server
EthernetClient client;
*/

typedef std::function<void(void)> MyFunc;

typedef MicroTuple<int, MyFunc> IF_1;
typedef MicroTuple<int, float> IF_2;

/** TODO
    - stocker dans un fichier la date des reboot
      fait
    - verifier l'effet des connections au ws
    - faire servir les fichiers html et js par le NUC , en fait j'ai relenti le chargement de code.js ( code_boot.js)
*/

const long utcOffsetInSeconds = 19800;
const long SEC_MS = 1000;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

WiFiClient client;

void fff() {
}

void (*reset)(void) = 0;

IF_2 ddd(1, 3.2);
IF_1 ddd1(1, fff);

 
// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
//const char* ssid = "Tenda-2.4G-ext"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

//const String IPADRESS="176.161.19.7";
const String WURL = String("http://") + String(IPADDRESS) + ":" + String(__PORT);


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO14, PIN=D5 on board
long PORTE=D1; 			// digital out pour le relais de la porte

long PORTE_OUVERTE = D5; 	// digital in with internal pullup
long PORTE_FERMEE = D6;  	// digital in with internal pullup

long SERIAL_RX = D7; 		// used with (swapped) Serial
long SERIAL_TX = D8; 		// used with (swapped) Serial

String buf_serial;
int swapped(false);

long load_page_num = 0;

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
   D7		GPIO13	OK	        OK	                SPI (MOSI) RX0                                                          ENA
   D8		GPIO15	pulled to GND	OK	                SPI (CS), Boot fails if pulled HIGH, TX0                                 HCC
   RX		GPIO03	OK	        RX pin	                HIGH at boot                                                            Trigger                
   TX		GPIO01	TX pin	        OK	                HIGH at boot, debug output at boot, boot fails if pulled LOW            
   A0		ADC0	Analog Input	X	
*/

/**

RX/TX (1,3) sont connectée à l'usb, donc pas possible d'utiliser Serial pour le debug et pour un autre communication a la fois
solutions : 
  OTA pour changer le code à la volée
  utiliser serial swap pour diriger Serial sur les pins 13/15 (RX/TX  = D7 / D8)
 
 */



//ESP8266WebServer server(80);




struct ARequest {
  using FS = fs::FS;
  void send(int code, const String &s, const String &ss) {
    auto p = (AsyncWebServerRequest*)this;
    p->send(code, s, ss);
  }
  void send(FS &fs, const String &s, const String &ss) {
    auto p = (AsyncWebServerRequest*)this;
    p->send(fs, s, ss);
  }
};

typedef void (*CB)(ARequest *);
typedef void (*CB1)(AsyncWebServerRequest *request);


struct AServer : AsyncWebServer {
  AServer(int p) : AsyncWebServer(p) {}
  void send2(int code, const String &t, const String &s) {
    //req->send(code, t, s);
  }
  void on(const String &u, const WebRequestMethod &m, const CB &cb) {
    AsyncWebServer::on(u.c_str(), m, (CB1&)cb);
  }
};

//AsyncWebServer server(80);
AServer server(80);
#if WS==1
AsyncWebSocket ws("/ws");
#endif

AsyncWebSocketClient * globalClient(NULL);;


struct EKOPrinterWS : EKOPrinter {
  virtual void println(const String &ss) {
    if (globalClient != NULL) {
      globalClient->text(ss);
    }
    {
      EKOPrinter::println(ss);
    }
  }
};

// linky
int IdxDataRawLinky = 0;
int IdxBufferLinky = 0;

const int DATA_LINKY = 1000;
const int BUFFER_LINKY = 30;
char DataRawLinky[DATA_LINKY];
char BufferLinky[BUFFER_LINKY];
//Internal Timers
unsigned long previousWifiMillis;
unsigned long previousWatchdogMillis;
unsigned long previousHistoryMillis;

bool LFon = false;

float Iinst = 0;  //I instantané
float Imoy = 0;   // I moyen sur 5mn
float Papp = 0;   //PVA instantané
float PappM = 0;  //PVA moyen sur 5mn
float tabI[600];
float tabP[600];
int IdxStock = 0;
long HCHC = 0;
long HCHP = 0;
long HCHC_last = 0;
long HCHP_last = 0;
int tabHC[600];
int tabHP[600];
int PWHP = 0;
int PWHC = 0;




#define UPLOAD_FILE 1
#if UPLOAD_FILE == 1

#include "favicon.h"
const unsigned char *favicon = bin2c_favicon_ico;
int favicon_length = sizeof(bin2c_favicon_ico) / sizeof(char);

//NTPClient *timeClient(NULL);

/* pour héberger les pages html et js
 * pourait plutôt être mise sur des fichier du file system de l'ESP8266 ( mais demande un méchanisme de upload)
 * autre solution : mettre ces pages sur autre serveur
 */

#include "code_boot.h"
String jscode_boot((const char*)bin2c_code_boot_js);
#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);
#include "gitinfo.h"
String GITINFO = (const char*)bin2c_gitinfo_txt;

#endif


long startTime = millis();
auto timeWritten = false;

void swap() {
  //delay(500);
  //interrupts();
  //delay(500);  
  //Serial.begin(115200); //Begin Serial at 115200 Baud

  push_message(Message(SWAP));
  //delay(500);
  //Serial.swap();
  //delay(500);

}

bool porte_ouverte() {
  // true : contact ouvert
  int d = digitalRead(PORTE_OUVERTE);
  //EKOX(d);
  return !d;
}

bool porte_fermee() {
  // true : contact ouvert  
  int d = digitalRead(PORTE_FERMEE);
  //EKOX(d);
  return !d;
}


void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //EKOX(type);
  if(type == WS_EVT_CONNECT){
    globalClient = client;
    EKOT("Websocket client connection received");
 
  } else if(type == WS_EVT_DISCONNECT){
    EKOT("disconnect");
    EKOT("Websocket client connection finished");
    globalClient = NULL;
  } else if(type == WS_EVT_DATA){
    //EKOX(len);
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len) {
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
      EKOX(msg.c_str());
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

void update_file(bool reboot = false, bool inc = false) {
  {
    String fn = "log.txt";
    auto ss = split("");
    {
      File file = LittleFS.open(fn, "r");
      if (file != 0) {
        file.close();
        ss = split(read_file("log.txt"), ",");
      }
    }
    auto s0 = ss.get<0>();
    auto s1 = ss.get<1>();
    EKOX(s0);
    EKOX(s1);
    auto n = s0.toInt() + (reboot ? 1 : 0);
    File file = LittleFS.open(fn, "w");
    //EKOX(file);
    int a = inc ? 1 : 0;
    if (file != 0) {
      //EKO();
      file.print(String(n) + "," + String(s1.toInt() + a));
      //EKO();
      file.close();
      //EKO();
    }
    EKO();
    //EKOX(read_file("log.txt"));
  }
}

void IRAM_ATTR putLow()
{
  digitalWrite(PORTE, LOW);
  EKO();
  
}

void setup() {
  
  if (WS)
    eko_printer = new EKOPrinterWS();

  pinMode(SERIAL_TX, OUTPUT);
  pinMode(SERIAL_RX,INPUT);
  
  pinMode(PORTE_OUVERTE,INPUT);
  pinMode(PORTE_FERMEE,INPUT);  

  Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  delay(500);
  Serial.print("\n\n");
  EKOT("starting");
  delay(6000);


  //////////////////////////////////////
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  delay(100);
  String aa = "|/-\\|/-\\";
  for (int i = 0; ; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\n");      
      break;
    }
    delay(500);
    //Serial.print("\u0008");    Serial.print(aa[i%aa.length()]);
    Serial.print(".");
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
    
  }
  
  EKOT(" wifi ok");
  EKOX(WiFi.localIP().toString());  //Print the local IP

  auto url = "www.google.com";
  IPAddress remote_addr;  
  EKOX(WiFi.hostByName(url, remote_addr));
  auto connect_ = client.connect(url, 80);
  EKOX(connect_);
  
  
  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr);  //Print the local IP

  //ArduinoOTA.begin(); //initOTA();
  
  
  // Initialize SPIFFS
  if(!LittleFS.begin()){
    EKOT("An Error has occurred while mounting SPIFFS");
  } else {
    EKOT("SPIFFS ok");

    create_file("favicon.ico", favicon, favicon_length);
    create_file("code.js", jscode);
    create_file("page.html", page);
  }

  
  jscode.replace("RANDOM", String(random(255)));
  jscode.replace("PORT", String(__PORT));
  jscode.replace("IPADDRESS", String(IPADDRESS));
  jscode.replace("WURL", WURL);
  jscode.replace("MODE", String(MODE));



#if WS==1
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  EKO();
#endif

  server.on("/identify", HTTP_GET, [](ARequest *request){
    String npage("garage");
    request->send(200, "text/json", nnpage.c_str());    
  });

  server.on("/create_file", HTTP_GET, [](ARequest *request){
    create_file("test.txt", "test");
    create_file("code.js", jscode);
    String npage(page);
    npage.replace("WURL", WURL);
    create_file("page.html", npage);
    String nnpage = String("{") + G("status") + " : " + G("ok") +  " }";
    request->send(200, "text/json", nnpage.c_str());    
  });
  
  server.on("/create_file", HTTP_GET, [](ARequest *request){
    create_file("test.txt", "test");
    create_file("code.js", jscode);
    String npage(page);
    npage.replace("WURL", WURL);
    create_file("page.html", npage);
    String nnpage = String("{") + G("status") + " : " + G("ok") +  " }";
    request->send(200, "text/json", nnpage.c_str());    
  });
  server.on("/read_file", HTTP_GET, [](ARequest *request){

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
  server.on("/reset", HTTP_GET, [](ARequest *request){
    reset();
  });

  server.on("/gitinfo", HTTP_GET, [](ARequest *request){
    String npage(GITINFO);
    request->send(200, "text/html", npage.c_str());
  });
    
  server.on("/main96713", HTTP_GET, [](ARequest *request){
    //EKOX(long(request));
    start = count;
    //EKOT("handle_index_main");
    int v = ledv ? LOW : HIGH;
    ledv = !ledv;
    EKOT("high");    
    digitalWrite(PORTE, HIGH);


    if (true) {
      tasks::ITimer.detachInterrupt();
      tasks::ITimer.attachInterruptInterval(500 * 1000, putLow);
    }


    
    if (false) {
      EKO();
      delay(500);
      digitalWrite(PORTE, LOW);
      EKO();
    }
    if (false) {
      tasks::apres(2 * tasks::SEC_MC, [](){
        EKOT("low");
        digitalWrite(PORTE, LOW);
      });
    }
    EKOX(tasks::dump());
    update_file(false, true);
    auto json = Acc(P("status", "ok"));
    //EKOX(json);
    request->send(200, "text/json", json);
    
    EKOT("end");
  });
  
  server.on("/code.js", HTTP_GET, [](ARequest *request){
    EKOT("js");
    String npage(jscode);
    EKO();
    EKOX(npage.length());
    request->send(200, "text/javascript", npage);
    EKOT("end");
  });

  server.on("/code_boot.js", HTTP_GET, [](ARequest *request){
    EKOT("code boot js");
    String npage(jscode_boot);
    EKO();
    EKOX(npage.length());
    request->send(200, "text/javascript", npage);
    EKOT("end");
  });

  server.on("/favicon.ico", HTTP_GET, [](ARequest *request){
    EKOT("icon");
    request->send(LittleFS, "/favicon.ico", "image/x-icon");
    EKOT("end");
  });
    
  server.on("/", HTTP_GET, [](ARequest *request){
    noInterrupts();
    
    EKOT("index");
    int a0 = digitalRead(PORTE_OUVERTE);
    EKOX(a0);
    EKOX(digitalRead(PORTE_FERMEE));
    String porte;
    porte += String(porte_ouverte() ? "ouverte" : "_");
    porte += " ";
    porte += String(porte_fermee() ? "fermee" : "_");
    
    String npage(page);
    npage.replace("DATE", String(DATE));
    npage.replace("WURL", WURL);
    npage.replace("PORTE", porte);
    npage.replace("LOADNUM", String(load_page_num));

    //npage.replace("code.js", "http://192.168.1.40/www/EPortier/code.js");

    
    //npage.replace("MESSAGE", "Compilation date : " + String(DATE) + ", loaded " + String(load_page_num) + " times.");    
    npage.replace("MESSAGE", "");
    load_page_num += 1;    
    //EKOT(npage);
    
    request->send(200, "text/html", npage.c_str());
    EKOT("end");
    interrupts();    
  });

  
  server.on("/swap", HTTP_GET, [](ARequest *request) {
    swap();
    String npage("{");
    npage += S + G("status") + " : " + G("ok") + ",";
    npage += S + G("swapped") + " : " + G(swapped);
    npage += " }";
    
    noInterrupts();
    EKOT("called swap");
    EKOX(swapped);
    
    request->send(200, "text/json", npage.c_str());
    interrupts();
  });
  
  server.on("/statut_porte", HTTP_GET, [](ARequest *request) {
    noInterrupts();    
    //EKOT("statut");
    String message = "POST form was:\n";
    //for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    //Serial.println(message);
    String statO = S + (porte_ouverte() ? "" : "pas") + " ouverte";
    String statF = S + (porte_fermee() ? "" : "pas") + " fermée";
    String json = Acc(P("porte_ouverte", statO) + ", " +
                      P("porte_fermee", statF));
    EKOX(json);
    request->send(200, "text/json", json);
    //EKOX(long(globalClient));
    EKO();

    if (false) {
      tasks::apres(tasks::SEC_MC /2, [](){
        EKOT("low");
        //digitalWrite(PORTE, LOW);
      });
      EKOX(tasks::dump());
    }
    interrupts();    
      
  });

  server.on("/data_linky", HTTP_GET, [](ARequest *request) {
    EKOT("data linky");
    noInterrupts();
    String json = Acc(P("papp", String(Papp)) + ", " +
                      P("pappm", String(PappM)) + ", " +                      
                      P("Iinst", String(Iinst)) + ", " +                     
                      P("Imoy", String(Imoy)) + ", " +                   
                      P("pwhp", String(PWHP)) + ", " +                     
                      P("pwhc", String(PWHC)));
    request->send(200, "text/json", json);
    interrupts();
    EKOX(json);
  });

  EKO();
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
  EKOX(DOSWAP);
  EKO();
  if (DOSWAP)  {
    EKO();
    EKOT("send message to swap");
    push_message(Message(SWAP));
  }
  EKO();
  //Timers
  previousWifiMillis = millis();
  previousWatchdogMillis = millis();
  previousHistoryMillis = millis();

  delay(1000);
  EKO();
  if (false) {
    File file = LittleFS.open("log.txt", "r");
    if (file != 0) {
      String s;
      while (file.available()) {
        auto c = file.read();
        s += String((char) c);
      }
      EKOX(s);
      file.close();
    } else {
      EKOT("pas trouvé");
    }
  }

  if (false) {
    
    File file = LittleFS.open("log.txt", "r");
    assert(file != 0);    
    String s;
    while (file.available()) {
      auto c = file.read();
      s += String((char) c);
    }
    EKOX(s);
    file.close();
    /*
    file = LittleFS.open("log.txt", "w");
    assert(file != 0);
    file.print(s + "\n" + "1");
    file.close();
    */
  }  
  EKO();
  listAllFilesInDir("/");
  
  EKO();

  /*
  // Define NTP Client to get time
  WiFiUDP *ntpUDP = new WiFiUDP();

  timeClient = new NTPClient(*ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
  timeClient->begin();
  */


  if (false) {
    
    auto t = tasks::apres(tasks::SEC_MC * 2, [](){
      {
        EKO();
        auto ss = read_file("log.txt");
        File file = LittleFS.open("/log.txt", "w");
        EKOX(file);
        if (file != 0) {
          file.print(ss + "une fois\n");
          file.close();
        }
      }
      
      EKO();
    });
  }

  
  
  update_file(true, false);
  
  startTime = millis();  
  EKOT("c'est parti");

  //tasks::test1();
  //tasks::test();

  
}
/*
auto o1 = Once([]() {
  update_file();

 }, 4000);
*/




// LINKY
//********
void LectureLinky() {  //Lecture port série du LINKY
  while (Serial.available() > 0) {
    int V = Serial.read();
    //EKOX(V);
    if (V == 2) {  //STX (Start Text)
      for (int i = 0; i < 5; i++) {
        DataRawLinky[IdxDataRawLinky] = '-';
        IdxDataRawLinky = (IdxDataRawLinky + 1) % DATA_LINKY;
      }
      digitalWrite(LED_BUILTIN, LOW);
    }
    if (V == 3) {  //ETX (End Text)
      digitalWrite(LED_BUILTIN, HIGH);
      //Reset du Watchdog . Il faut des messages du Linky
      if (millis() - previousWatchdogMillis > 3000) {
        //esp_task_wdt_reset();
        previousWatchdogMillis = millis();
      }
    }
    if (V > 9) {  //Autre que ETX et STX
      switch (V) {
        case 10:  // Line Feed. Debut Groupe
          LFon = true;
          IdxBufferLinky = 0;
          break;
        case 13:       // Fin Groupe
          if (LFon) {  //Debut groupe OK
            LFon = false;
            int nb_blanc = 0;
            String code = "";
            String val = "";
            for (int i = 0; i < IdxBufferLinky; i++) {
              if (BufferLinky[i] == ' ') {
                nb_blanc++;
              }
              if (nb_blanc == 0) {
                code += BufferLinky[i];
              }
              if (nb_blanc == 1) {
                val += BufferLinky[i];
              }
              if (nb_blanc < 2) {  //On ne prend pas le check somme, uniquement 2 premier champs
                DataRawLinky[IdxDataRawLinky] = BufferLinky[i];
                IdxDataRawLinky = (IdxDataRawLinky + 1) % 1000;
              }
            }
            DataRawLinky[IdxDataRawLinky] = char(13);
            IdxDataRawLinky = (IdxDataRawLinky + 1) % 1000;
            //EKOX(code);
            if (code.indexOf("IINST") == 0) {
              Iinst = val.toFloat();
              //EKOX(Iinst);
              if (Imoy == 0) { Imoy = Iinst; }
              Imoy = float(Iinst + 149 * Imoy) / 150;  //moyenne courant efficace 5 dernieres minutes environ
              
              //EKOX(Imoy);
            }
            if (code.indexOf("PAPP") == 0) {
              Papp = val.toFloat();
              if (PappM == 0) { PappM = Papp; }
              PappM = (Papp + 149 * PappM) / 150;  //moyenne puissance apparente 5 dernieres minutes environ
              //EKOX(PappM);
            }
            if (code.indexOf("HCHP") == 0 || code.indexOf("BASE") == 0) {
              HCHP = val.toInt();
              //EKOX(HCHP);
            }
            if (code.indexOf("HCHC") == 0) {
              HCHC = val.toInt();
              //EKOX(HCHC);
            }
          }
          break;
        default:
          BufferLinky[IdxBufferLinky] = char(V);
          IdxBufferLinky = (IdxBufferLinky + 1) % 30;
          break;
      }
      //EKOX(char(V));
      //Debug.print(char(V));
    }
  }
 
}
long last = 0;


void loop() {
  //ArduinoOTA.handle();
  long now = millis();
  /*
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
    EKO();
  }
  */
  //o0();
  
  if (false && (now - startTime) > 10 * SEC_MS) {
    EKOX(now - startTime);
    EKOX(now);
    EKOX(startTime);
    EKOT("resetting");
    delay(500);
    //reset();
  }

  //server.handleClient();

  if (globalClient != NULL && globalClient->status() == WS_CONNECTED) {
    //EKO();
    delay(500);
    /*
      auto r = random(0,100); 
      if (r == 1) {
      String randomNumber = String(random(0,100));
      globalClient->text(randomNumber);
      }
    */
  }
  delay(4);    
  
  if (messages_available()) {
    EKO();
    auto m = take_message();
    if (m.c == SWAP) {
      EKO();
      swapped = !swapped;
      EKOX(swapped);
      if (swapped) {
        // bascule le port serie rx/tx sur D7/D8    
        // linky
        EKOT("swapping to 1200");
        //delay(500);    
        Serial.begin(1200, SERIAL_7E1);
        //EKO();
        EKOT("swapped to 1200")    ;
      } else {
        EKOT("swapping to 115200");    
        Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
        EKO();
      }
      EKOT("SWAPPING");
      Serial.swap();
      EKO();
    }
  }

  
  if (swapped) {
    //EKOX(swapped);
    //delay(500);
    LectureLinky();
  }
  if (now > (last + 1000)) {
    last = now;
    //EKOX(now);
    //String ipaddr = WiFi.localIP().toString(); 
    //EKOX(ipaddr);  //Print the local IP
    
  }
  //EKO();
  count += 1;
}
