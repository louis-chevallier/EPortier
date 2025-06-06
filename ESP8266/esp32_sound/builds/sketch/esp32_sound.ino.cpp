#include <Arduino.h>
#line 1 "/home/louis/dev/git/EPortier/ESP8266/esp32_sound/esp32_sound.ino"
#include "util.h"
#include "SPI.h"

#include "MCP_ADC.h"

MCP3208 mcp28;
#define MCP3208_CS_PIN 25
long start, stop, analog_read_time;
long analog_read_multiple_time;

const uint8_t num_channels = 8;
uint8_t channels_list[num_channels] = {
    0,1,2,3,4,5,6,7
};



#line 18 "/home/louis/dev/git/EPortier/ESP8266/esp32_sound/esp32_sound.ino"
void setup();
#line 47 "/home/louis/dev/git/EPortier/ESP8266/esp32_sound/esp32_sound.ino"
void test();
#line 102 "/home/louis/dev/git/EPortier/ESP8266/esp32_sound/esp32_sound.ino"
void loop();
#line 18 "/home/louis/dev/git/EPortier/ESP8266/esp32_sound/esp32_sound.ino"
void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  delay(1000);
  Serial.print("starting\n");
  EKO();

  SPI.begin();

  Serial.println();
  Serial.println("ADC\tCHAN\tMAXVALUE");
  Serial.print("mcp28\t");
  Serial.print(mcp28.channels());
  Serial.print("\t");
  Serial.println(mcp28.maxValue());

  Serial.println("***************************************\n");
  for (int s = 1; s <= 16; s++)
  {
    Serial.println(s * 1000000UL);
    mcp28.setSPIspeed(s * 1000000UL);
    mcp28.begin(MCP3208_CS_PIN);
    test();
  }

  
}



void test()
{
  uint32_t val = 0;

  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.read(channel);
  }
  stop = micros();
  analog_read_time = stop - start;
  
  Serial.print("mcp28.read()\t8x: \t");
  Serial.println(analog_read_time);
  delay(10);


  start = micros();
  int16_t readings[num_channels];
  
  mcp28.readMultiple(channels_list, num_channels, readings);
  stop = micros();
  analog_read_multiple_time = stop - start;

  Serial.print("mcp28.readMultiple()\t8x: \t");
  Serial.println(stop - start);

  Serial.print("read() time / readMultiple() time \t");
  Serial.println((1.0 * analog_read_time) / analog_read_multiple_time);
  delay(10);


  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.differentialRead(channel);
  }
  stop = micros();
  Serial.print("mcp28.differentialRead() 8x: \t");
  Serial.println(stop - start);
  delay(10);

  start = micros();
  for (int channel = 0; channel < mcp28.channels(); channel++)
  {
    val += mcp28.deltaRead(channel);
  }
  stop = micros();
  Serial.print("mcp28.deltaRead()\t8x: \t");
  Serial.println(stop - start);
  Serial.println();
  delay(10);

}

void loop() {
  delay(1); 
}

