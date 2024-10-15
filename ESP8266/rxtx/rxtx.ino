// à include avant asyncweb, sinon, ca crash
#include <FS.h>
#include "LittleFS.h"

#include "ESPAsyncWebServer.h"
#include "microTuple.h"
#include "ESP8266TimerInterrupt.h"

#include "util.h"
#include "tasks.h"

long count = 0;
int ledv = 1>2;
long start = 0;


long SERIAL_RX = D7; 		// used with (swapped) Serial
long SERIAL_TX = D8; 		// used with (swapped) Serial

String buf_serial;
bool swapped(false);
const int K = 100;
char buf[K];
long load_page_num = 0;

const bool receiver = true;
const bool emitter = !receiver;

String padded(long i, int  p) {
  String s(i);
  while (s.length() < p) {
    s = String("0") + s;
  }
  return s;
}


void setup() {
  EKOT("starting");  
  //pinMode(SERIAL_RX,INPUT);
  //Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  delay(2000);  
  Serial.begin(115200, SERIAL_8N1);
  delay(6000);
  if (true) {
    //noInterrupts();  
    //interrupts();
    Serial.begin(1200, SERIAL_7E1);
    if (emitter) {
      pinMode(SERIAL_TX, OUTPUT);
    } else {
      delay(6000);      
      EKOT("passe en mode acquisition ...");
      delay(1000);
      pinMode(SERIAL_RX, INPUT);      
    }
    Serial.swap();
  }
  
  if (emitter) {
    for (int i = 0; ; i++) {
      delay(1000);
      String s = padded(i, 3) + "A";
      Serial.print(s);
    }
  }
}

void loop()  {
  if (receiver) {
    if (count < K) {
      if (Serial.available() > 0) {
        int c = Serial.read();
        buf[count ++] = c;
      }
    } else if (count == K) {
      delay(3000);
      Serial.swap();
      Serial.begin(115200, SERIAL_8N1);
      delay(3000);
      EKOT("received !!");
      for (int i = 0; i < count; i++) {
        Serial.print(buf[i]);
      }
      delay(1000);
      Serial.begin(1200, SERIAL_7E1);
      Serial.swap();
      delay(1000);
      count = 0;
    }
  }
}
