#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "microTuple.h"

typedef std::function<void(void)> MyFunc;

typedef MicroTuple<int, MyFunc> IF_1;
typedef MicroTuple<int, float> IF_2;

void fff() {
}


IF_2 ddd(1, 3.2);
IF_1 ddd1(1, fff);


void onremove(IF_1 &ff) {
}

ESP8266WebServer server(80);

String S;

long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()

const String ROOT_URL = String("http://") + String(IPADDRESS) + ":" + String(PORT);
const String WURL = ROOT_URL + "/main";
  
// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);

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


void setup() {
  

  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  delay(500);
  String sss = "test.ino:69: [450ms]";
  for (int i = 0; i < sss.length(); i++) {
    EKOX(int(sss[i]));
  }
  EKOX(sss);

  EKO();
  //pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  //digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  page.replace("JSCODE", jscode);


  WiFi.begin(ssid, password);
  //WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
    delay(500);
    EKOT("Waiting to connect...");
  }
  String ipaddr = WiFi.localIP().toString(); 
  EKOX(ipaddr); 
  server.on("/main", HTTP_GET, [](){
    //EKO();
    String npage("{");
    npage += S + "\"swapped\" : " + G(swapped) ;
    npage += S + "," + G("status") + " : " + G("ok");
    npage += S + "," + G("buf_serial") + " : " + G(buf_serial.length());
    npage += " }";
    //EKOX(npage);
    server.send(200, "text/json", npage.c_str());
  });
  server.on("/swap", HTTP_GET, [](){
    EKO();
    swap();
    String npage("{");
    npage += S + G("status") + " : " + G("ok");
    npage += " }";
    server.send(200, "text/json", npage.c_str());
  });
  server.on("/", HTTP_GET, [](){
    EKO();
    String npage(page);
    npage.replace("PORT", String(PORT));
    npage.replace("IPADDRESS", String(IPADDRESS));
    npage.replace("WURL", WURL);
    npage.replace("ROOT_URL", ROOT_URL);    
    server.send(505, "text/html", npage.c_str());

  });
  server.begin();
  EKOT("started");
}

int last = 0;



void loop() {
  server.handleClient();
  auto now = millis();
  if (swapped) {
    if (Serial.available()) {
      char c = Serial.read();
      buf_serial += String(c);
    }
  } else {
    if (buf_serial.length() > 0) {
      EKOX(buf_serial.length());
      String ss;
      for (int i = 0; i < buf_serial.length(); i++) {
        //EKOX(int(buf_serial[i]));
        //EKOX(String(buf_serial[i]));
        if (buf_serial[i] < 254) {
          if (buf_serial[i] <= 13) {
            ss += "\n";
          } else
            ss += String(buf_serial[i]);
        } 
      }
      EKOX(ss);

      
      buf_serial = "";
    }
  }

  
  if (now > last + 1000) {
    last = now;
    EKO();
  }
}
