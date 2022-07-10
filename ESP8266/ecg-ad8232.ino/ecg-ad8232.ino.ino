
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



ESP8266WebServer server(80);

// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
//const char* WURL = "http://176.161.19.7:8080/main";
// deuxieme mcu 
//const char* WURL = "http://176.161.19.7:8081/main";
// troisieme mcu 
const char* WURL = "http://176.161.19.7:8082/main";

// chez pepito
//const char* ssid = "Bbox-09179E72"; //Enter Wi-Fi SSID
//const char* password =  "114564D1FA44C45A736FF6AE6D5E3C"; //Enter Wi-Fi Password
//const char* WURL = "http://176.161.19.7:8080/main";


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO15, PIN=D8 on board
long PINOUT=15;
int buttonPin = 3;
void handle_index_main() {
  start = count;
  Serial.print("handle_index_main");
  //Print Hello at opening homepage
  String message("count =");
  message += String(count);
  server.send(200, "text/html", message.c_str());
  //"Hello! This is an index page.");
  int v = ledv ? LOW : HIGH;
  ledv = !ledv;
  Serial.print("end");
}

 
 String page(R""""(
<!DOCTYPE html>
<html>
 <head>
  <meta charset="utf-8"/>
  <script type="application/javascript">
    function draw() {
      const canvas = document.getElementById('canvas');
      if (canvas.getContext) {
        const ctx = canvas.getContext('2d');
        const x = new Float32Array([ FLOATS]);
        ctx.fillStyle = 'rgb(200, 0, 0)';
        ctx.fillRect(10, 10, 50, 50);

        ctx.fillStyle = 'rgba(0, 0, 200, 0.5)';
        ctx.fillRect(30, 30, 50, 50);
      }
    }
  </script>
 </head>
 <body onload="draw();">
   <canvas id="canvas" width="150" height="150"></canvas>
 </body>
</html>
)"""");

const int BUFFER_SIZE=10000; // 10000 => 10.7 sec (30.815 - 20.147)
float buffer[BUFFER_SIZE];
float sample_per_sec = 10000/(30.815 - 20.147);


void fill_buffer() {
  Serial.println("reading..");
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = analogRead(A0);
    delay(1);
  }
  Serial.println("end readng");
}
void handle_index() {
  Serial.print("index");
  String a0 = String(analogRead(A0));
  Serial.print(a0);
  String npage(page);
  fill_buffer();
  String floats(buffer[0]);
  Serial.println("making message");
  for (int i = 1; i < BUFFER_SIZE; i++) {
    floats = floats + "," + String(buffer[i]);
  }
  npage.replace("FLOATS", floats);
  Serial.println("SENDING");
 
  server.send(505, "text/html", npage.c_str());
  Serial.print("end");
}

void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.println("setup...");
  Serial.println(String("sample rate...") + String(sample_per_sec));
  
  //pinMode(10, INPUT); // Setup for leads off detection LO +
  //pinMode(11, INPUT); // Setup for leads off detection LO -
  // INPUT ANALOG
  pinMode(A0,INPUT);

  {
    delay(10);
    WiFi.begin(ssid, password);  //Connect to the WiFi network
  
    while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
        delay(500);
       Serial.println("Waiting to connect...");
    }
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //Print the local IP

    pinMode(A0,INPUT);

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.println(WiFi.localIP());
    server.on("/", handle_index); //Handle Index page
    server.on("/main96713", handle_index_main); //Handle Index page
  
    server.begin(); //Start the server
    Serial.println("setup");
    pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    digitalWrite(2, HIGH);  
 
    //pinMode(PINOUT, OUTPUT);
    //digitalWrite(PINOUT, LOW);  
    Serial.println("Server listening");
  }
  Serial.print("end setup");
}



void loop4() {
  Serial.println("loop");
  fill_buffer();
  Serial.println("filled");
}
 
  
void loop3() {
  Serial.println("loop");
  if((digitalRead(10) == 1)||(digitalRead(11) == 1)){
    Serial.println('!');
  }
  else {
    // send the value of analog input 0:
    Serial.println(analogRead(A0));
  }
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
// loop checks the button pin each time,
// and will send serial if it is pressed
void loop2() {
  if (digitalRead(buttonPin) == HIGH) {
    Serial.write('H');
  }
  else {
    Serial.write('L');
  }

  delay(1000);
}
void loop() {
  server.handleClient(); //Handling of incoming client requests
  count += 1;

}
