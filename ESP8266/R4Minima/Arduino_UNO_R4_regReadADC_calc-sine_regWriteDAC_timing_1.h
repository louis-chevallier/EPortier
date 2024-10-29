/*  Arduino UNO R4 test code for fast non-blocking ADC analog-read, and DAC analog-write operation
 *  With fast sine calculation of frequency from ADC value
 *
 *  Susan Parker - 2nd November 2023.
 *    Realtime sine calc with Paul Stoffregen's 11th order Taylor Series Approximation
 *    https://www.pjrc.com/high-precision-sine-wave-synthesis-using-taylor-series/
 *    Note: I have condensed the Taylor Series code to a single set of inline asm calls
 *
 * This code is "AS IS" without warranty or liability. 
 * There may be glitches, etc. please let me know if you find any; thanks.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
*/

// ARM-developer - Accessing memory-mapped peripherals
// https://developer.arm.com/documentation/102618/0100

// Low Power Mode Control - See datasheet section 10

#include "util.h"

#include "register.h"



// === Local Defines

// #define ADC_EXT_AREF         // Use external ADC Aref source
// #define ADC_AVARAGE          // Enable 4x averaging i.e. four sucessive conversions
#define ADSSTR00               // Enable Sampling State Register change from default
#define ADSSTR00_VAL 0x08      // A/D Sampling State Register 0 - Default is 0x0D

#define SINE_DAC


bool tick_tock = false;

long count = 0;

typedef int16_t D;
const D buffer_size = 13000 / sizeof(D);
D buffer[buffer_size];

uint16_t analog_read_value = 0;
uint16_t analog_read_value_1 = 0;
uint16_t analog_write_value;
int16_t result_sine;
int32_t result_taylor_sine;
void loop1()
{
  int32_t out, a , b;

  static int32_t sine_amp_local = 0x7FFFFFFF;
  static uint32_t freq_time_accumulate = 0;
  //  static uint32_t freq_time_delta = 0x1 << 24;  //  
  static uint32_t freq_time_delta = 0x1 << 18;  // This value gives 

    
  int32_t angle, sum, p1, p2, p3, p5, p7, p9, p11, term; 
  uint32_t ph;

  {
    *PFS_P103PFS_BY = 0x05;      // Pulse on D4 to trigger scope 
    *PFS_P103PFS_BY = 0x04;      // Each Port Output bit clear or set takes c. 83nS 
  }
  
  if (1) {
    //  *PFS_P107PFS_BY = 0x05;      //  
    analog_read_value = *ADC140_ADDR00;    // adcValue = analogRead(analogPin); // Internal 16bit register read = c. 123nS 
    analog_read_value_1 = *ADC140_ADDR01;    // adcValue = analogRead(analogPin); // Internal 16bit register read = c. 123nS 
    *ADC140_ADCSR |= (0x01 << 15);  // Next ADC conversion = write to register c. 300nS
    //  *PFS_P107PFS_BY = 0x04;      //
  }
#ifdef SINE_DAC
  {
#ifdef ADC_RIGHT_JUST
    freq_time_accumulate += (uint32_t)analog_read_value << 16;   // Core DDS frequency-synthesis incriment
#else
    freq_time_accumulate += (uint32_t)analog_read_value << 14;   // Core DDS frequency-synthesis incriment
#endif

    //Serial.print(EKOX(analog_read_value);
    //EKOX(freq_time_accumulate);

    if (freq_time_accumulate >= 0xC0000000u || freq_time_accumulate < 0x40000000u)
      {
        angle = (int32_t)freq_time_accumulate; // valid from -90 to +90 degrees
      } 
    else
      {
        angle = (int32_t)(0x7FFFFFFFu - freq_time_accumulate);
      }
    term = angle << 1;
    asm volatile("smmulr %0, %1, %2" : "=r" (p1) : "r" (term), "r" (1686629713));
    asm volatile("smmulr %0, %1, %2" : "=r" (term) : "r" (p1), "r" (p1));
    p2 = term << 3;
    asm volatile("smmulr %0, %1, %2" : "=r" (term) : "r" (p2), "r" (p1));
    p3 = term << 3;
    term = p1 << 1;
    asm volatile("smmlsr %0, %2, %3, %1" : "=r" (sum) : "r" (term), "r" (p3), "r" (1431655765));
    asm volatile("smmulr %0, %1, %2" : "=r" (term) : "r" (p3), "r" (p2));
    p5 = term << 1;
    asm volatile("smmlar %0, %2, %3, %1" : "=r" (sum) : "r" (sum), "r" (p5), "r" (286331153));
    asm volatile("smmulr %0, %1, %2" : "=r" (p7) : "r" (p5), "r" (p2));
    asm volatile("smmlsr %0, %2, %3, %1" : "=r" (sum) : "r" (sum), "r" (p7), "r" (54539267));
    /*  
    // Uncommet this for 25bit precision calculation
    asm volatile("smmulr %0, %1, %2" : "=r" (p9) : "r" (p7), "r" (p2));
    asm volatile("smmlar %0, %2, %3, %1" : "=r" (sum) : "r" (sum), "r" (p9), "r" (6059919));
    asm volatile("smmulr %0, %1, %2" : "=r" (p11) : "r" (p9), "r" (p2));
    asm volatile("smmlsr %0, %2, %3, %1" : "=r" (sum) : "r" (sum), "r" (p11), "r" (440721));
    */

    result_taylor_sine = sum << 1;   

    //	asm volatile("smmulr %0, %1, %2" : "=r" (out) : "r" (result_taylor_sine), "r" (sine_amp_local));
    //  result_sine = (int16_t)(out >> 19);

#ifdef DAC_RIGHT_JUST
    
    result_sine = (int16_t)(result_taylor_sine >> 20);   // 32 - 12 = 20
    *DAC12_DADR0 = result_sine + 2048;
#else
    result_sine = (int16_t)(result_taylor_sine >> 16);   // 
    // result_sine in [ -32767, 32767]

    *DAC12_DADR0 = result_sine + 32767;
    //EKOX(result_sine);
    
#endif
  }
#else
  {


    if (0) {
#ifdef ADC_RIGHT_JUST
      analog_read_value = analog_read_value >> 2;  // 
      analog_write_value = (~analog_read_value & 0x0FFF);  // do this outside the DAC timed window
#else
      analog_write_value = (~analog_read_value);  // do this outside the DAC timed window
#endif
    }
    if (0) {
      if(tick_tock == true)
        {
          *PFS_P107PFS_BY = 0x05;      // Set HIGH
          *DAC12_DADR0 = analog_read_value;  // 
          *PFS_P107PFS_BY = 0x04;      // Read plus Set LOW = c. 250nS
          tick_tock = false;
        }
      else
        {
          *PFS_P107PFS_BY = 0x05;      // Set HIGH
          *DAC12_DADR0 = analog_write_value;  // 
          *PFS_P107PFS_BY = 0x04;      // Read plus Set LOW = c. 250nS
          tick_tock = true;
        }
    }
  }
#endif
  //  delayMicroseconds(1000);  // cant use delay() because AGT0 is stopped
}

void (*reset)(void) = 0;

void loop() {
  loop1();
  count += 1;
  buffer[count % buffer_size] = analog_read_value;

  if (count == 1) {
    EKO();
  }
  if (count == 1000000 * 2) {
    EKOX(analog_write_value);
    EKOX(analog_read_value);
    for (int i = 0; i < buffer_size; i++) {
      Serial.println(buffer[i]);
    }
    EKOT("end");
    //delay(1000);
    //reset();
  }
}


void setup_adc(void)
{
  *MSTP_MSTPCRD &= (0xFFFFFFFF - (0x01 << MSTPD16));  // Enable ADC140 module
#ifdef ADC_EXT_AREF
  *ADC140_ADHVREFCNT = 0x01;         // Set External Aref = analogReference(AR_EXTERNAL);      
#endif
#ifdef ADC_RIGHT_JUST
  *ADC140_ADCER = 0x06;              // 14 bit mode, right justified
#else
  *ADC140_ADCER = 0x8006;            // 14 bit mode, left-justified
  //  *ADC140_ADCER = 0x8000;            // 12 bit mode, left-justified
#endif
  *ADC140_ADANSA0 |= (0x01 << 0);    // Selected ANSA00 = A1 as DAC is on A0
#ifdef ADC_AVARAGE
  *ADC140_ADADC    = 0x83;           // Average mode - 4x and b7 to enable averaging
  *ADC140_ADADS0  |= (0x01 << 0);    // Enable Averaging for ANSA00 channel
#endif
#ifdef ADSSTR00
  *ADC140_ADSSTR00 = ADSSTR00_VAL;   // A/D Sampling State Register 0 - Default is 0x0D
#endif
}

void setup_dac(void)       // Note make sure ADC is stopped before setup DAC
{
  *MSTP_MSTPCRD &= (0xFFFFFFFF - (0x01 << MSTPD20));  // Enable DAC12 module
#ifdef DAC_RIGHT_JUST
  *DAC12_DADPR    = 0x00;        // DADR0 Format Select Register - Set right-justified format
#else
  *DAC12_DADPR    = 0x80;        // DADR0 Format Select Register - Set left-justified format i.e. 16 bit format, 4 LSBs not used
#endif
  //  *DAC12_DAADSCR  = 0x80;        // D/A A/D Synchronous Start Control Register - Enable
  *DAC12_DAADSCR  = 0x00;        // D/A A/D Synchronous Start Control Register - Default
  // 36.3.2 Notes on Using the Internal Reference Voltage as the Reference Voltage
  *DAC12_DAVREFCR = 0x00;        // D/A VREF Control Register - Write 0x00 first - see 36.2.5
  *DAC12_DADR0    = 0x0000;      // D/A Data Register 0 
  delayMicroseconds(10);        
  *DAC12_DAVREFCR = 0x01;        // D/A VREF Control Register - Select AVCC0/AVSS0 for Vref
  //  *DAC12_DAVREFCR = 0x03;        // D/A VREF Control Register - Select Internal reference voltage/AVSS0
  //  *DAC12_DAVREFCR = 0x06;        // D/A VREF Control Register - Select External Vref; set VREFH&L pins used for LEDs
  *DAC12_DACR     = 0x5F;        // D/A Control Register - 
  delayMicroseconds(5);         // 
  *DAC12_DADR0    = 0x0800;      // D/A Data Register 0 
  *PFS_P014PFS   = 0x00000000;   // Make sure all cleared
  *PFS_P014PFS  |= (0x1 << 15);  // Port Mode Control - Used as an analog pin
}

void setup() {
  Serial.begin(115200);
  //  while (!Serial){};

  setup_adc();
  setup_dac();

  *ADC140_ADCSR   |= (0x01 << 15);   // Start an ADC conversion
  delayMicroseconds(10);
  *AGT0_AGTCR = 0;                     // disable Millis counter, delay etc. don't want this Interrupt messing up output stability

  for (int i = 0; i < buffer_size; i++) {
    buffer[i] = i;
  }
  EKOX(buffer_size);
  
}
