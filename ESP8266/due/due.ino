

#include "util.h"

void setup ()
{

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  
  // uncomment as required
  //  ADCSRA |= bit (ADPS0);                               //   2  
  //  ADCSRA |= bit (ADPS1);                               //   4  
  //  ADCSRA |= bit (ADPS0) | bit (ADPS1);                 //   8  
  ADCSRA |= bit (ADPS2);                               //  16 
  //  ADCSRA |= bit (ADPS0) | bit (ADPS2);                 //  32 
  //  ADCSRA |= bit (ADPS1) | bit (ADPS2);                 //  64 
  //  ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);   // 128
  
  Serial.begin (115200);
  Serial.println ();
  
}  // end of setup

const int ITERATIONS = 10000;
unsigned long totals [6];
const byte lowPort = 0;
const byte highPort = 4;

void loop ()
{
  for (int whichPort = lowPort; whichPort <= highPort; whichPort++)
    totals [whichPort - lowPort] = 0;
  
  unsigned long startTime = micros ();
  for (int i = 0; i < ITERATIONS; i++)
    {
      for (int whichPort = lowPort; whichPort <= highPort; whichPort++)
        {
          int result = analogRead (whichPort);
          totals [whichPort - lowPort] += result;
        } 
    }
  unsigned long endTime = micros ();
  
  for (int whichPort = lowPort; whichPort <= highPort; whichPort++)
    {
      Serial.print ("Analog port = ");
      Serial.print (whichPort);
      Serial.print (", average result = ");
      Serial.println (totals [whichPort - lowPort] / ITERATIONS);
    }
  auto time_taken = endTime - startTime; 
  EKOX(time_taken);

  auto sample_time_ms =  float(time_taken)/ITERATIONS/1000;
  EKOX(sample_time_ms);
  auto sample_rate = 1./sample_time_ms*1000;
  EKOX(sample_rate);
  Serial.println ();
  Serial.flush ();
  exit (0);
}
// end of loop
