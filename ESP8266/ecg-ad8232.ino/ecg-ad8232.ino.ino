
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <ESP8266TimerInterrupt.h>
#include "QueueArray.h"

ESP8266WebServer server(80);

// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
//const char* WURL = "http://176.161.19.7:8080/main";
// deuxieme mcu 
//const char* WURL = "http://176.161.19.7:8081/main";

// troisieme mcu 
const char* WURL = "http://176.161.19.7:8082/main";
const char* url = "http://176.161.19.7:8082/fetch";

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
int sample_freq=2000;
long count_irq = 0;

const int BUFFER_SIZE=1000; // 10000 => 10.7 sec (30.815 - 20.147)
float buffer[BUFFER_SIZE];
float sample_per_sec = 10000/(30.815 - 20.147);

QueueArray<float> queue;
int transmission_started = 1>2;
int consumed = 0;

typedef struct Pair { 
  long unsigned int l; float v;
  Pair(const long unsigned int &pl = 0, const float &pv = 0) : l(pl), v(pv) {}
  operator String() const { return String(l) + ":" + v; }
};
QueueArray<Pair> queuep;


#define TIMER_INTERVAL_MS       1000

// Init ESP8266 timer 1
ESP8266Timer ITimer;

volatile bool statusLed = false;
volatile uint32_t lastMillis = 0;

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
        const x = new Float32Array([ 0.1, 0.3]);
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
    <div>
       <button class="bb", id="lancer">Lancer l'ECG</button>
    </div>

   <script type="application/javascript">
    const button = document.getElementById("lancer");
    received = 0;
    chunks = 0;
    function lancer(debut, duree) {
        url = "espData?";
        url += String("duree=") + String(duree);
        url += String("&debut=") + String(debut);
        //console.log("fetching");
        fetch(url)
          .then((response) => response.json())
          .then((data) => {
            chunks += 1;
            //console.log(data.pairs);
            received += data.pairs;
            if (received < 2000) {
               lancer(12, 13); 
            } else {
              console.log("received " + chunks);
            }
          }).catch(console.error)

    }
 
    button.addEventListener('click', function() {
      received = 0;
      rep  = lancer(10, 33);
      console.log(rep); 
    });

    
   </script>
 </body>
</html>
)"""");


void fill_buffer() {
  Serial.println("reading..");
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = analogRead(A0);
    delay(1);
  }
  Serial.println("end readng");
}

void handle_index() {
  Serial.println("index");
  //String a0 = String(analogRead(A0));
  //Serial.print(a0);
  String npage(page);
 
  Serial.println("SENDING");
 
  server.send(505, "text/html", npage.c_str());
  Serial.println("end sending");
}

void handle_fetch() {
  Serial.println("fetch");
  String a0 = String(analogRead(A0));
  //Serial.print(a0);
  String npage(page);
  fill_buffer();
  String floats(buffer[0]);
  Serial.println("making message");
  for (int i = 1; i < BUFFER_SIZE; i++) {
    floats = floats + "," + String(buffer[i]);
  }
  Serial.println("SENDING");
 
  server.send(200, "text/html", floats.c_str());
  Serial.println("end sending");
}

void IRAM_ATTR TimerHandler()
{
  float v = analogRead(A0);
  if (queuep.count() > 20) {
    queuep.pop();
  } 
  queuep.push(Pair(millis(), v));
}

void ICACHE_RAM_ATTR ISR(){
  //timer0 interrupt 2kHz toggles pin 8
  //generates pulse wave of frequency 2kHz/2 = 1kHz (takes two cycles for full wave- toggle high then toggle low)

  count_irq = (count_irq + 1) % sample_freq;
}

void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.println("setup...");
  Serial.println(String("sample rate...") + String(sample_per_sec));


  if (0) {
    timer1_attachInterrupt(ISR);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);  
  /* Dividers:
    TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
    TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
    TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
    Reloads:
    TIM_SINGLE  0 //on interrupt routine you need to write a new value to start the timer again
    TIM_LOOP  1 //on interrupt the counter will start with the same value again
  */
    timer1_write(31249); //100 ms // for fclk=80MHz  (fclck/prescale*t)-1  Register 32bit?
  }

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

    
    server.on("/", handle_index); //Handle Index page
    server.on("/fetch", handle_fetch); //Handle Index page

    server.on("/espData", HTTP_GET, []() {
      String message = "POST form was:\n";
      //Serial.println("aaa " + String(millis()));
      //Serial.println("lll" + String(queuep.count()));
     
      for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
      //Serial.println(message);

      transmission_started = 1>0;
      String floats = "[" + queue.dump() + "]";      
      floats  = "[0,1]";
      String json;
      json = "{\"time\":";
      json += millis();

      json += ", \"floats\":";
      json += floats;

      json += ", \"pairs\" : " + String(queuep.count());  //[ " + queuep.dump() + "]";      

      
      json += ", \"heap\":";
      json += ESP.getFreeHeap();
      json += ", \"analog\":";
      json += analogRead(A0);
      json += "}";
      server.send(200, "text/json", json);
      //Serial.println("bbb " + json);
      queuep.empy();
      //Serial.println("bbb " + String(millis()));

  });


  
  if (0==1) {
    if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1, TimerHandler))
    {
      lastMillis = millis();
      Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(lastMillis);
    }
    else
      Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));
  }

    server.begin(); //Start the server
    Serial.println("setup");
    pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    digitalWrite(2, HIGH);  
 
    //pinMode(PINOUT, OUTPUT);
    //digitalWrite(PINOUT, LOW);  
    Serial.println("Server listening");
  }
  Serial.println("end setup");
  

  Serial.println("sizeof " + String(sizeof(Pair)));
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

unsigned int lastM = 0;
void loop() {
  //Serial.println(queue.count());
  server.handleClient(); //Handling of incoming client requests
  //Serial.println("apres handle " + String(millis()));
  count += 1;

  if (micros() > lastM + 10000 ) {
    float v = analogRead(A0);
    lastM = micros();
    //Serial.println("queue L" + String(queuep.count()));
    if (queuep.count() > 2000) {
        //Serial.println("overflow");
        queuep.dequeueA();
    } 
    queuep.enqueueA(Pair(micros(), v));
 }

  
  if (0) {
    if (!transmission_started and !queue.isEmpty()) {
      queue.pop();
    }
    queue.push(analogRead(A0));
    if (queue.count() % 4000 == 0) {
      Serial.println("queue count" + String(queue.count()));
    }
  }
}
