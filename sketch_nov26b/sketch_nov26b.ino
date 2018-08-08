
// GPIO port 0
#define PORT0_DIR 0xf202
#define PORT0_DATA 0xf200
// GPIO port 1
#define PORT1_DIR 0xf206
#define PORT1_DATA 0xf204
// GPIO port 2
#define PORT2_DIR 0xf20A
#define PORT2_DATA 0xf208
// GPIO port 3
#define PORT3_DIR 0xf20E
#define PORT3_DATA 0xf20C
// GPIO port 4
#define PORT4_DIR 0xf212
#define PORT4_DATA 0xf210
// GPIO port 5
#define PORT5_DIR 0xf216
#define PORT5_DATA 0xf214
// GPIO port 6
#define PORT6_DIR 0xf21A
#define PORT6_DATA 0xf218
// GPIO port 7
#define PORT7_DIR 0xf21E
#define PORT7_DATA 0xf21C
// GPIO port 8
#define PORT8_DIR 0xf222
#define PORT8_DATA 0xf220
// GPIO port 9
#define PORT8_DIR 0xf226
#define PORT8_DATA 0xf224

#include "Arduino.h"
#include <unistd.h>
#include <pc.h>
#include <string.h>


unsigned long time1, time2, microseconds;
int i, printonce;

void setup() {
Serial.begin(9600);
i = 0;
printonce = 1;
}

void loop() {
    if (printonce)
    {
          time1 = micros();
          outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)
          outportb(PORT1_DIR, 0xff); // set DIR register to 0xff (set all pins of GPIO port 0 are OUTPUT)
          do
          {
                 outportb(PORT1_DATA, 5); // set DATA register to  (eByte1)
                 outportb(PORT2_DATA, 10); // set DATA register to  (eByte2)             
                 i++;
          } while (i < 10000);
          time2 = micros();
    }
      
    microseconds = time2-time1;
    Serial.print("Time in microseconds bare minimum = : ");
    Serial.print(microseconds);
    Serial.println("");
    printonce = 0;
}
