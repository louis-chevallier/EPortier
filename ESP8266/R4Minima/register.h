#pragma once

#define SYSTEM 0x40010000 // System Registers
#define SYSTEM_SBYCR   ((volatile unsigned short *)(SYSTEM + 0xE00C))      // Standby Control Register
#define SYSTEM_MSTPCRA ((volatile unsigned int   *)(SYSTEM + 0xE01C))      // Module Stop Control Register A

#define MSTP 0x40040000 // Module Registers
#define MSTP_MSTPCRB   ((volatile unsigned int   *)(MSTP + 0x7000))      // Module Stop Control Register B
#define MSTPB2   2 // CAN0
#define MSTPB8   8 // IIC1
#define MSTPB9   9 // IIC0
#define MSTPB18 18 // SPI1
#define MSTPB19 19 // SPI0
#define MSTPB22 22 // SCI9
#define MSTPB29 29 // SCI2
#define MSTPB30 30 // SCI1
#define MSTPB31 31 // SCI0

#define MSTP_MSTPCRC   ((volatile unsigned int   *)(MSTP + 0x7004))      // Module Stop Control Register C
#define MSTP_MSTPCRD   ((volatile unsigned int   *)(MSTP + 0x7008))      // Module Stop Control Register D
#define MSTPD2   2 // AGT1   - Asynchronous General Purpose Timer 1 Module
#define MSTPD3   3 // AGT0   - Asynchronous General Purpose Timer 0 Module
#define MSTPD5   5 // GPT320 and GPT321 General 32 bit PWM Timer Module
#define MSTPD6   6 // GPT162 to GPT167 General 16 bit PWM Timer Module
#define MSTPD14 14 // POEG   - Port Output Enable for GPT Module Stop
#define MSTPD16 16 // ADC140 - 14-Bit A/D Converter Module
#define MSTPD19 19 // DAC8   -  8-Bit D/A Converter Module
#define MSTPD20 20 // DAC12  - 12-Bit D/A Converter Module
#define MSTPD29 29 // ACMPLP - Low-Power Analog Comparator Module
#define MSTPD31 31 // OPAMP  - Operational Amplifier Module

// The Mode Control bits are read as 1, the write value should be 1.
// Bit value 0: Cancel the module-stop state 
// Bit value 1: Enter the module-stop state.

// ====  Asynchronous General Purpose Timer (AGT) =====
#define AGTBASE 0x40084000 
#define AGT0_AGTCR    ((volatile unsigned char  *)(AGTBASE + 0x008))  // AGT Control Register

// =========== ADC14 ============
// 35.2 Register Descriptions
#define ADCBASE 0x40050000 /* ADC Base */

#define ADC140_ADCSR   ((volatile unsigned short *)(ADCBASE + 0xC000)) // A/D Control Register
#define ADC140_ADANSA0 ((volatile unsigned short *)(ADCBASE + 0xC004)) // A/D Channel Select Register A0
#define ADC140_ADANSA1 ((volatile unsigned short *)(ADCBASE + 0xC006)) // A/D Channel Select Register A1
#define ADC140_ADADS0  ((volatile unsigned short *)(ADCBASE + 0xC008)) // A/D-Converted Value Addition/Average Channel Select Register 0
#define ADC140_ADADS1  ((volatile unsigned short *)(ADCBASE + 0xC00A)) // A/D-Converted Value Addition/Average Channel Select Register 1
#define ADC140_ADCER   ((volatile unsigned short *)(ADCBASE + 0xC00E)) // A/D Control Extended Register 
#define ADC140_ADSTRGR ((volatile unsigned short *)(ADCBASE + 0xC010)) // A/D Conversion Start Trigger Select Register
#define ADC140_ADEXICR ((volatile unsigned short *)(ADCBASE + 0xC012)) // A/D Conversion Extended Input Control Register
#define ADC140_ADANSB0 ((volatile unsigned short *)(ADCBASE + 0xC014)) // A/D Channel Select Register B0
#define ADC140_ADANSB1 ((volatile unsigned short *)(ADCBASE + 0xC016)) // A/D Channel Select Register B1
#define ADC140_ADTSDR  ((volatile unsigned short *)(ADCBASE + 0xC01A)) // A/D conversion result of temperature sensor output
#define ADC140_ADOCDR  ((volatile unsigned short *)(ADCBASE + 0xC01C)) // A/D result of internal reference voltage
#define ADC140_ADRD    ((volatile unsigned short *)(ADCBASE + 0xC01E)) // A/D Self-Diagnosis Data Register

#define ADC140_ADDR00 ((volatile unsigned short *)(ADCBASE + 0xC020))      // A1 (P000 AN00 AMP+)
#define ADC140_ADDR01 ((volatile unsigned short *)(ADCBASE + 0xC020 +  2)) // A2 (P001 AN01 AMP-) 
#define ADC140_ADDR02 ((volatile unsigned short *)(ADCBASE + 0xC020 +  4)) // A3 (P002 AN02 AMPO) 
#define ADC140_ADDR05 ((volatile unsigned short *)(ADCBASE + 0xC020 + 10)) // Aref (P010 AN05 VrefH0)
#define ADC140_ADDR09 ((volatile unsigned short *)(ADCBASE + 0xC020 + 18)) // A0 (P014 AN09 DAC)
#define ADC140_ADDR21 ((volatile unsigned short *)(ADCBASE + 0xC040 + 10)) // A4 (P101 AN21 SDA) 
#define ADC140_ADDR22 ((volatile unsigned short *)(ADCBASE + 0xC040 + 12)) // A5 (P100 AN20 SCL) 

#define ADC140_ADHVREFCNT ((volatile unsigned char  *)(ADCBASE + 0xC08A)) // A/D High-Potential/Low-Potential Reference Voltage Control Register
#define ADC140_ADADC      ((volatile unsigned char  *)(ADCBASE + 0xC00C)) // A/D-Converted Value Addition/Average Count Select Register

#define ADC140_ADSSTR00 ((volatile unsigned char *)(ADCBASE + 0xC0E0))      // AN00 A/D Sampling State Register

// 12-Bit D/A Converter
#define DACBASE 0x40050000          // DAC Base - DAC output on A0 (P014 AN09 DAC)
#define DAC12_DADR0    ((volatile unsigned short *)(DACBASE + 0xE000))      // D/A Data Register 0 
#define DAC12_DACR     ((volatile unsigned char  *)(DACBASE + 0xE004))      // D/A Control Register
#define DAC12_DADPR    ((volatile unsigned char  *)(DACBASE + 0xE005))      // DADR0 Format Select Register
#define DAC12_DAADSCR  ((volatile unsigned char  *)(DACBASE + 0xE006))      // D/A A/D Synchronous Start Control Register
#define DAC12_DAVREFCR ((volatile unsigned char  *)(DACBASE + 0xE007))      // D/A VREF Control Register


// =========== Ports ============
// 19.2.5 Port mn Pin Function Select Register (PmnPFS/PmnPFS_HA/PmnPFS_BY) (m = 0 to 9; n = 00 to 15)
#define PORTBASE 0x40040000 /* Port Base */

#define P000PFS 0x0800  // Port 0 Pin Function Select Register
#define PFS_P000PFS ((volatile unsigned int *)(PORTBASE + P000PFS))            // 
#define PFS_P001PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 1 * 4))) // 
#define PFS_P002PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 2 * 4))) // 
#define PFS_P003PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 3 * 4))) // 
#define PFS_P004PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 4 * 4))) // 
#define PFS_P005PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 5 * 4))) // 
#define PFS_P006PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 6 * 4))) // 
#define PFS_P007PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 7 * 4))) // 
#define PFS_P008PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 8 * 4))) // 
// #define PFS_P009PFS ((volatile unsigned int *)(PORTBASE + P000PFS + ( 9 * 4))) // Does not exist
#define PFS_P010PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (10 * 4))) // 
#define PFS_P011PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (11 * 4))) // 
#define PFS_P012PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (12 * 4))) // 
#define PFS_P013PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (13 * 4))) // N/C
#define PFS_P014PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (14 * 4))) // N/A
#define PFS_P015PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (15 * 4))) // N/A

#define PFS_P100PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843))   // 8 bits - A5
#define PFS_P101PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 1 * 4))) // A4
#define PFS_P102PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 2 * 4))) // D5
#define PFS_P103PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 3 * 4))) // D4
#define PFS_P104PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 4 * 4))) // D3
#define PFS_P105PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 5 * 4))) // D2
#define PFS_P106PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 6 * 4))) // D6
#define PFS_P107PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 7 * 4))) // D7
#define PFS_P108PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 8 * 4))) // SWDIO
#define PFS_P109PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 9 * 4))) // D11 / MOSI
#define PFS_P110PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (10 * 4))) // D12 / MISO
#define PFS_P111PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (11 * 4))) // D13 / SCLK
#define PFS_P112PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (12 * 4))) // D10 / CS
#define PFS_P300PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3))            // SWCLK (P300)
#define PFS_P301PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (01 * 4))) // D0 / RxD (P301)
#define PFS_P302PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (02 * 4))) // D1 / TxD (P302) 
#define PFS_P303PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (03 * 4))) // D9
#define PFS_P304PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (04 * 4))) // D8

