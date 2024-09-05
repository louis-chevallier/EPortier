long seko = millis();

String S;

#define EKOT(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + String(x) + "."); seko=millis()
#define EKOX(x) Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms] " + #x + "=" + String(x) + "."); seko=millis()
#define EKO()   Serial.println(S + __FILE__ + ":" + String(__LINE__) + ": [" + String(millis()-seko) + "ms]"); seko=millis()



void setup() {
  
  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  delay(10);

  EKO();
  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);
  
}

int last = 0;

void loop() {
     auto now = millis();
     if (now > last + 1000) {
       last = now;
        EKO();
     }
}
