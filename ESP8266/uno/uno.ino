
void setup ()
{
  Serial.begin (115200);
  Serial.println ();
  
}  // end of setup

const int ITERATIONS = 1000;
unsigned long totals [6];
const byte lowPort = 0;
const byte highPort = 3;

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
  Serial.print ("Time taken = ");
  Serial.print (endTime - startTime);
  
  Serial.println ();
  Serial.flush ();
  exit (0);
}  // end of loop
