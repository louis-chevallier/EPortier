#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


ESP8266WebServer server(80);

const char* ssid = "CHEVALLIER_BORDEAU_F"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO15, PIN=D8 on board
long PINOUT=15;

void handle_index_main() {
  start = count;
  Serial.print("main");
  //Print Hello at opening homepage
  String message("count =");
  message += String(count);
  server.send(200, "text/html", message.c_str());
  //"Hello! This is an index page.");
  int v = ledv ? LOW : HIGH;
  ledv = !ledv;
  digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  digitalWrite(PINOUT, HIGH); 
  delay(2000);
  digitalWrite(2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(PINOUT, LOW); 
}


 String page(
"<!DOCTYPE html>\n"

"<html>\n"
"  <head>\n"
"<meta http-equiv=GGGContent-typeGGG content=GGGtext/html; charset=utf-8GGG />\n"
"    <title>Ouverture de la porte</title>\n"
"    <style>\n"
"    .bb {\n"
"        font-size: 100px;\n"
"    }\n"
"    </style>\n"
"  </head>\n"
"  <body>\n"
"  <div>\n"
"    <button class=GGGbbGGG, id=GGGouvrirGGG>Ouvrir</button>\n"
"  </div>\n"
"    <script>\n"
"      const button = document.getElementById(GGGouvrirGGG);\n"
"          reset = function(){\n"
"                                        button.style.fontSize=GGG100pxGGG;"
"                                        button.innerHTML = GGGOuvrirGGG;\n"
"                                        button.disabled = false;\n"
"                                };\n"
"          const url = GGGhttp://78.207.134.29:8083/mainGGG\n"
"          button.addEventListener('click', () => {\n"
"                                        button.style.fontSize=GGG30pxGGG;"
"                                        button.innerHTML = GGGle verrou va s'ouvrir ...GGG;\n"
"                setTimeout(() => {\n"
"                        button.disabled = true;\n"
"                        window.fetch(url, { mode: 'no-cors'}).then((result) => {\n"
"                                button.style.fontSize=GGG30pxGGG;"
"                                button.innerHTML = GGGSystème contacté,<br>la porte est déverrouillée pendant 2 secondes ...GGG;\n"
"                                setTimeout(reset, 2000);\n"
"                        }).catch((e) => {\n"
"                                button.innerHTML = GGGPas moyen de contacter le système! ;)..GGG;\n"
"                                setTimeout(reset, 2000);\n"
"                        }\n"
"                        )\n"
"                        },\n"
"                        1000)})\n"
"    </script>\n"
"  </body>\n"
"</html>\n"

);

void handle_index() {
  Serial.print("index");

  page.replace("GGG", "\"");
 
  server.send(505, "text/html", page.c_str());
}

void setup() {
  
  Serial.begin(115200); //Begin Serial at 115200 Baud
  delay(10);
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  server.on("/", handle_index); //Handle Index page
  server.on("/main", handle_index_main); //Handle Index page
  
  server.begin(); //Start the server
  Serial.println("Server listening");
  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  
  pinMode(PINOUT, OUTPUT);
  digitalWrite(PINOUT, LOW);  
}

void loop() {
  server.handleClient(); //Handling of incoming client requests
  count += 1;

}
