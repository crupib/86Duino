
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

byte eLUT[4][3]; //Current Encoder Val: 0-3, Encoder Count Inc: -1, 0, 1
byte  AB[8];      //(8) quad encoder outputs, (4) motor and (4) image
int eInc[8];       //(8) increment buffers, (4) motor and (4) image
int eCts[8];      //(8) encoder output counters
byte eByte1, eByte2; //port(1) and port(2) storage bytes
long MaxMtr, MaxImg; //Set which Motor axis is master, and which Image axis is master
unsigned long time1, time2, microseconds;
int i, printonce;
void setup() {
eLUT[0][0]= 1; //AB OLD: 00 / AB NEW: 01
eLUT[1][0]= 3; //AB OLD: 01 / AB NEW: 11
eLUT[2][0]= 0; //AB OLD: 10 / AB NEW: 00
eLUT[3][0]= 2; //AB OLD: 11 / AB NEW: 10

//Increment = 0 AB
eLUT[0][1]= 0; //AB OLD: 00 / AB NEW: 00 = same, no change
eLUT[1][1]= 1; //AB OLD: 01 / AB NEW: 01 = same, no change
eLUT[2][1]= 2; //AB OLD: 10 / AB NEW: 10 = same, no change
eLUT[3][1]= 3; //AB OLD: 11 / AB NEW: 11 = same, no change

//Increment = +1 AB
eLUT[0][2]= 2; //AB OLD: 00 / AB NEW: 10
eLUT[1][2]= 0; //AB OLD: 01 / AB NEW: 00
eLUT[2][2]= 3; //AB OLD: 10 / AB NEW: 11
eLUT[3][2]= 1; //AB OLD: 11 / AB NEW: 01
Serial.begin(9600);
i = 0;
printonce = 1;
}

void loop() {
  // put your main code here, to run repeatedly:
//Generate Look-up tables - one time only
//Increment = -1 AB
//'*********************************************************************************
//'Timer code, buffer code, path code, etc., located here
//'*********************************************************************************
   
    if (printonce)
        {
           time1 = micros();
           outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)
           outportb(PORT1_DIR, 0xff); // set DIR register to 0xff (set all pins of GPIO port 0 are OUTPUT)
           MaxMtr = 1;
           MaxImg = 1;
                  do
                  {
                   //'*********************************************************************************
                   //'Timer code, buffer code, path code, etc., located here
                   //'*********************************************************************************
                          if (MaxMtr)  //'max motor counter <> 0, write to port
                          {
                                AB[0]= eLUT[AB[0]][eInc[0]+1]; // 'using LUT, translate AB, based on current increment
                                AB[1]= eLUT[AB[1]][eInc[1]+1];
                                AB[2]= eLUT[AB[2]][eInc[2]+1];
                                AB[3]= eLUT[AB[3]][eInc[3]+1];
                                eByte1= AB[0] | AB[1] << 2 | AB[2] <<4 | AB[3] <<6; //'combine to byte value
                                outportb(PORT1_DATA, eByte1); // set DATA register to  (eByte1)
                                eCts[0]+= eInc[0]; //'update counters
                                eCts[1]+= eInc[1];
                                eCts[2]+= eInc[2];
                                eCts[3]+= eInc[3];
                         }
                         // Image output port
                         if (MaxImg)
                         {
                                AB[4]= eLUT[AB[4]][eInc[4]+1]; // 'using LUT, translate AB, based on current increment
                                AB[5]= eLUT[AB[5]][eInc[5]+1];
                                AB[6]= eLUT[AB[6]][eInc[6]+1];
                                AB[7]= eLUT[AB[7]][eInc[7]+1];
                                eByte2= AB[4] | AB[5] <<2 | AB[6]<<4 | AB[7]<<6;// 'combine to byte value                     
                                outportb(PORT2_DATA, eByte2); // set DATA register to  (eByte2)
                                eCts[4]+= eInc[4]; //'update counters
                                eCts[5]+= eInc[5];
                                eCts[6]+= eInc[6];
                                eCts[7]+= eInc[7];
                        }
                        i += 1;
                  } while (i < 10000);
                  time2 = micros();
    }
      
      microseconds = time2-time1;
      Serial.print("Time in microseconds = : ");
      Serial.print(microseconds);
      Serial.println("");
      printonce = 0;
}
