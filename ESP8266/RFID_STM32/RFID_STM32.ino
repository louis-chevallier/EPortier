#include <NiusWireless.h>
#include "util.h"



#define RC522_I2C_ADDR 0x28   // 0x29 if I2C_ADD0 is tied HIGH on the board
#define RC522_RST_PIN  10

NiusRC522 rfid(Wire, RC522_I2C_ADDR, RC522_RST_PIN);

void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  delay(1000);
  Serial.print("starting\n");
  EKO();
  Serial.begin(9600);
  rfid.begin();
}

void loop() {
    if (!rfid.cardPresent()) return;
    rfid.printInfo();
    rfid.halt();
}

