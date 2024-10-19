#include <vector>
#include <algorithm>
#include "util.h"




auto BUFFER_SIZE=1000;
std::vector<float> buffer;


const int SIGNAL_FREQ = 10; // Hz
const auto SIGNAL_PERIOD_SEC = 1. / SIGNAL_FREQ; // Sec

const auto SAMPLING_FREQ = 1000.; // Hz
const auto SAMPLING_PERIOD_SEC = 1. / SAMPLING_FREQ;

auto r1 = Ramp(SIGNAL_PERIOD_SEC / 2 * 1000, 0, 1);
auto signal1 = rev(r1);
auto signal = repeat(cat(r1, rev(r1)), 100000);




void setup() {
  
  pinMode(A0,INPUT);

  //dac.begin(0x62);
  
  Serial.begin(115200); //Begin Serial at 115200 Baud
  EKOT("starting");
  EKOX(signal.duration_ms());
  delay(10);
  EKO();

  pinMode(A0,INPUT);

  EKOX(String(SAMPLING_PERIOD_SEC, 4));
  EKOX(SIGNAL_PERIOD_SEC);

  pinMode(D7,OUTPUT);

  
  // T = 10ms
  if (ITimer.attachInterruptInterval(SAMPLING_PERIOD_SEC * 1000000, TimerHandler)) {
    Serial.print(F("Starting  ITimer0 OK, millis() = "));
    Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer0. Select another Timer, freq. or timer"));

}

long sss(0);

long last = 0;

auto last_sample = micros();

auto stop = false;




void loop() {
  delay(1);
  
}
