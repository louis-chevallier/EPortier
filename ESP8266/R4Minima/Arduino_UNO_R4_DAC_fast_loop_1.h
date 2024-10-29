/*  Arduino UNO R4 Minima code for DAC12 demonstration
 *
 *  Susan Parker - 19th August 2023.
 *
 * This code is "AS IS" without warranty or liability. 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

// RA4M1 User’s Manual: Hardware
// This doc has all the register discriptions I use:
// https://www.renesas.com/us/en/document/mah/renesas-ra4m1-group-users-manual-hardware

// ARM-developer - Accessing memory-mapped peripherals
// https://developer.arm.com/documentation/102618/0100

#include "register.h"


uint16_t loop_count = 0;               // One 0 to 4095 ramp takes 2.7mS

void loop()                            // Total loop()          - takes c. 667nS per loop; or c. 750nS per loop with if()
  {
  *PFS_P107PFS_BY = 0x05;              // Set D7 output high    - takes c.  83nS

  *DAC12_DADR0 = loop_count++;         // DAC update            - takes c. 210nS  - DAC ignores top 4 bits

  *PFS_P107PFS_BY = 0x04;              // Set D7 output low     - takes c.  83nS
/*
  if(loop_count >= 4096)               // loop() and loop_count - takes c. 500nS
    loop_count = 0;                    // ... when test is true, adds c. 83nS to loop time to reset counter
*/
  }                                    // bare loop()           - takes c. 210nS


void setup_dac(void)       // Note make sure ADC is stopped before setup DAC
  {
  *MSTP_MSTPCRD &= ~(0x01 << MSTPD20);  // Enable DAC12 module
  *DAC12_DADPR    = 0x00;               // DADR0 Format Select Register - Set right-justified format
//  *DAC12_DAADSCR  = 0x80;               // D/A A/D Synchronous Start Control Register - Enable
  *DAC12_DAADSCR  = 0x00;               // D/A A/D Synchronous Start Control Register - Default
// 36.3.2 Notes on Using the Internal Reference Voltage as the Reference Voltage
  *DAC12_DAVREFCR = 0x00;               // D/A VREF Control Register - Write 0x00 first - see 36.2.5
  *DAC12_DADR0    = 0x0000;             // D/A Data Register 0 
   delayMicroseconds(10);               // Needed delay - see datasheet
  *DAC12_DAVREFCR = 0x01;               // D/A VREF Control Register - Select AVCC0/AVSS0 for Vref
  *DAC12_DACR     = 0x5F;               // D/A Control Register - 
   delayMicroseconds(5);                // 
  *DAC12_DADR0    = 0x0800;             // D/A Data Register 0 
  *PFS_P014PFS   = 0x00000000;          // Port Mode Control - Make sure all bits cleared
  *PFS_P014PFS  |= (0x1 << 15);         // ... use as an analog pin
  }

// Quick loop DAC output test

void setup()
  {                                                 
  *PFS_P107PFS_BY = 0x04;               // Set D7 output low - DAC time flag pin

  setup_dac();

  *AGT0_AGTCR = 0;                     // disable Millis counter, delay etc. don't want this Interrupt messing up output stability
  }
