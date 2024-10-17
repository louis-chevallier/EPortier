
#include "util.h"

#include <DueTimer.h>
#include "waveforms.h"
volatile int wave0 = 0;
int i = 0;

void myHandler_1()
{
  analogWrite(DAC0, waveformsTable_1[wave0]); 
  i++;
  if (i == maxSamplesNum)
    i = 0;
}
void setup()
{
  Serial.begin(9600);
  analogWriteResolution(12);
  NVIC_SetPriority(TC4_IRQn, 1);
  Timer4.attachInterrupt(myHandler_1).start(166.6666667); // DAC
}
void loop()
{
}
