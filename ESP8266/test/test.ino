#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


ESP8266WebServer server(80);

String S;

long seko = millis();
#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()

const String WURL = String("http://") + String(IPADDRESS) + ":" + String(PORT) + "/main";
  
// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password

#include "code.h"
String jscode((const char*)bin2c_code_js);
#include "page.h"
String page((const char*)bin2c_page_html);

void setup() {
  
  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  delay(10);

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
    EKO();
  });
  server.on("/", HTTP_GET, [](){
    EKO();
      String npage(page);
      npage.replace("PORT", String(PORT));
      npage.replace("IPADDRESS", String(IPADDRESS));
      npage.replace("WURL", WURL);
      
      server.send(505, "text/html", npage.c_str());
      

  });
  server.begin();
  EKOT("started");
}

int last = 0;

void loop() {
  server.handleClient();
  auto now = millis();
  if (now > last + 1000) {
    last = now;
    EKO();
  }
}
