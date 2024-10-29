#include "util.h"
#include "register.h"
#include <Seeed_Arduino_FS.h>
namespace adcdac {
#include "Arduino_UNO_R4_regReadADC_calc-sine_regWriteDAC_timing_1.h"
}

namespace dac {
#include "Arduino_UNO_R4_DAC_fast_loop_1.h"
}

const long utcOffsetInSeconds = 19800;
const long SEC_MS = 1000;

long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO14, PIN=D5 on board
long PORTE=D1; 			// digital out pour le relais de la porte

long PORTE_OUVERTE = D5; 	// digital in with internal pullup
long PORTE_FERMEE = D6;  	// digital in with internal pullup

long SERIAL_RX = D7; 		// used with (swapped) Serial
long SERIAL_TX = D7; 		// used with (swapped) Serial

String buf_serial;
bool swapped(false);

long load_page_num = 0;


typedef uint16_t D;
const D  s = 13000 / sizeof(D);
D buffer[s];


void setup() {
  EKOX(sizeof(int));
  Serial.begin(115200, SERIAL_8N1); //Begin Serial at 115200 Baud
  delay(5000);
  Serial.print("\nStarting\n\n");
  //analogWriteResolution(12);
  dac::setup_dac();
  EKO();

  EKOX(sizeof(D));
  EKOX(s);
  EKOT("c'est parti");
}


long last = 0;

void (*reset)(void) = 0;

float period;
float signal_freq = 5000; // 5 KHz
float signal_T = 1./ signal_freq;
long count_T;

long loop_count = 0;
void loop()                            // Total loop()          - takes c. 667nS per loop; or c. 750nS per loop with if()
{

  *PFS_P107PFS_BY = 0x05;              // Set D7 output high    - takes c.  83nS
  const long M = 4096;
  auto v = loop_count;
  auto v0 = v%M;
  auto v1 = max(v, M/2);
  *DAC12_DADR0 = v1;
  
  *PFS_P107PFS_BY = 0x04;              // Set D7 output low     - takes c.  83nS
  
  if(loop_count >= M)               // loop() and loop_count - takes c. 500nS
    loop_count = 0;                    // ... when test is true, adds c. 83nS to loop time to reset counter
  loop_count ++;
}                                    // bare loop()           - takes c. 210nS

