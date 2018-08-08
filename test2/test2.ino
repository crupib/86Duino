//
//  atco_rev3 - Sketch
//
//  Created by William Crupi on 11/28/15.
//  Copyright © 2015 William Crupi. All rights reserved.
//
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
#define PORT9_DIR 0xf226
#define PORT9_DATA 0xf224
typedef unsigned char byte;
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"
#include <unistd.h>
#include <pc.h>

//    '*******************************************************************************************
//    '  THIS CODE RUNS ONE TIME AT BEGINNING OF PROGRAM
//    '*******************************************************************************************

long  PathCycles, PathCnts, pCycle, pCnt;
double stp;
long Timer1Clk;
unsigned long time1, time2, microseconds;
int runloop;
long ctr, lctr; //loop counters
long PathOff, PathNext;
long TRUE;
unsigned char AB0,AB1,AB2,AB3,AB4,AB5,AB6,AB7;                        //'quad coded outputs
long eCts0,eCts1,eCts2,eCts3,eCts4,eCts5,eCts6,eCts7;        //'encoder counters
int eInc0,eInc1,eInc2,eInc3,eInc4,eInc5,eInc6,eInc7;         //'path increment
int pCts0,pCts1,pCts2,pCts3,pCts4,pCts5,pCts6,pCts7;         //'raw path count
int pCtsA0,pCtsA1,pCtsA2,pCtsA3,pCtsA4,pCtsA5,pCtsA6,pCtsA7; //'abs path count
int pCtsN0,pCtsN1,pCtsN2,pCtsN3,pCtsN4,pCtsN5,pCtsN6,pCtsN7; //'-path flag
//    '**************************************************************************************
//    ' ASSIGN VALUES TO ALL LOOKUP TABLES
//    '**************************************************************************************
//    'vLUT: lookup table, TRUE if value of any path inc equals 1
byte vLUT[2][2][2][2]= {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//    'sLUT: lookup table to set count to + or - sign
byte sLUT[2][2] = {1,1,2,0}; //'set count sign: -1=0,0=1,1=2
//    'eLUT: lookup table for setting encoder output code
byte eLUT[4][3] = {1,0,2,3,1,0,0,2,3,2,3,1}; //Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1
//    'pLUT: path cycle lookup table
byte pLUT[1001][1001];
//'bLUT: byte written to port LUT
byte bLUT[4][4][4][4] =
{
    0, 1, 2, 3,
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14, 15,
    
    16, 17, 18, 19,
    20, 21, 22, 23,
    24, 25, 26, 27,
    28, 29, 30, 31,
    
    32, 33, 34, 35,
    36, 37, 38, 39,
    40, 41, 42, 43,
    44, 45, 46, 47,
    
    48, 49, 50, 51,
    52, 53, 54, 55,
    56, 57, 58, 59,
    60, 61, 62, 63,
    
    
    64, 65, 66, 67,
    68, 69, 70, 71,
    72, 73, 74, 75,
    76, 77, 78, 79,
    
    80, 81, 82, 83,
    84, 85, 86, 87,
    88, 89, 90, 91,
    92, 93, 94, 95,
    
    96, 97, 98, 99,
    100, 101, 102, 103,
    104, 105, 106, 107,
    108, 109, 110, 111,
    
    112, 113, 114, 115,
    116, 117, 118, 119,
    120, 121, 122, 123,
    124, 125, 126, 127,
    
    
    128, 129, 130, 131,
    132, 133, 134, 135,
    136, 137, 138, 139,
    140, 141, 142, 143,
    
    144, 145, 146, 147,
    148, 149, 150, 151,
    152, 153, 154, 155,
    156, 157, 158, 159,
    
    160, 161, 162, 163,
    164, 165, 166, 167,
    168, 169, 170, 171,
    172, 173, 174, 175,
    
    176, 177, 178, 179, 
    180, 181, 182, 183, 
    184, 185, 186, 187, 
    188, 189, 190, 191, 
    
    
    192, 193, 194, 195, 
    196, 197, 198, 199, 
    200, 201, 202, 203, 
    204, 205, 206, 207, 
    
    208, 209, 210, 211, 
    212, 213, 214, 215, 
    216, 217, 218, 219, 
    220, 221, 222, 223, 
    
    224, 225, 226, 227, 
    228, 229, 230, 231, 
    232, 233, 234, 235, 
    236, 237, 238, 239, 
    
    240, 241, 242, 243, 
    244, 245, 246, 247, 
    248, 249, 250, 251, 
    252, 253, 254, 255    
};
void Check_pLUT()
{
    double stp;
    
    long PathCycle, PathCycles, PathCnts,  PathCnt, Ctr;
    
    PathCnts = 1000;
    PathCycles = PathCnts; //'always equal: 1 cycle per Path count
    byte pLUT[PathCnts+1][PathCycles+1];
    
    //'******************************************************************************
    //'Create the Look up table  - one time only, could stored and loaded from disk!
    memset(pLUT, 0, sizeof(pLUT[0][0]) * (PathCnts+1) * (PathCycles+1));
    for(PathCnt = PathCnts; PathCnt > 0; PathCnt--)
    {
        //stp = (PathCycles/PathCnt);
        stp = ((double)PathCycles/PathCnt);
        Ctr = 0;
        do
        {
            
            PathCycle = (long)(PathCycles - (stp * Ctr));
            if (PathCycle < 1)
                break;
            else
                pLUT[PathCnt][PathCycle] = 1;
            Ctr++;
        
        } while (1);
        
    }
    
    //'check results for errors
    Ctr=0;
    for (PathCnt = 1; PathCnt < PathCnts+1; PathCnt++)
    {
        for (PathCycle = 1; PathCycle < PathCycles+1; PathCycle++)
        {
            Ctr = Ctr + pLUT[PathCnt][PathCycle];
        }
        Serial.print("PathCnt = ");
        Serial.println(PathCnt);
        Serial.print("Ctr = ");
        Serial.println(Ctr);
        
        if (Ctr != PathCnt)
            ;//printf("Ctr != PathCnt\n");
        Ctr = 0;
    }
    
    Serial.println("pLUT Check Completed");
}

 

void setup()
{
    Serial.begin(9600);
    TRUE = 1;
    PathCycles = 1000;
    PathCnts = PathCycles; //'always equals PathCycles
    
    //'Path counts are spread out into the fixed number of path cycles.
    //'The path count must not exceed the fixed number of path cycles
    //'Set number of path cycles run continuous irrelevent of path counts
    //PathCycles = 400; //'default value, subject to change, user set   
    //'Maximum path counts equals the allowed limit any one path count can be.
    //'examples, Path Counts = 400; 400/400 = 1 count output per 1 cycle
    //'          Path Counts = 200; 400/200 = 1 count output per 2 cycles
    //'          Path Counts = 150; 400/150 = 1 count output per 2 or 3 cycles
    //'          Path Counts = 1;   400/1   = 1 count output per 400 cycles    
    Timer1Clk = 60 * PathCycles; //'Timer1 clock = 60Hz * 400 Pathcycles = 24KHz       
    //    'pLUT: generate lookup table, based on Path cycle and Path counts
    //    ' all varibles are longs except stp which is a float, double
    
    memset(pLUT, 0, sizeof(pLUT[0][0]) * (PathCnts+1) * (PathCycles+1));
    
    for (pCnt = PathCnts; pCnt > 0; pCnt--)
    {
        stp =(double)(PathCycles/pCnt);
        ctr = 0;
        do
        {
            pCycle = (long)(PathCycles - (stp*ctr));
            if (pCycle <1)
                break;
            else
                pLUT[pCnt][pCycle] = 1;
                //pLUT[pCycle][pCnt] = 1;
            ctr++;
        } while (1);
    }
    
    //    ' END OF STARTUP CODE
    //    '**************************************************************************************************
    outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)outportb(PORT2_DIR, 0xff);
    outportb(PORT1_DIR, 0xff); // set DIR register to 0xff (set all pins of GPIO port 0 are OUTPUT)
    PathOff = TRUE;
    PathNext = TRUE;
    runloop = 1;
    eCts0 = 0;
    eCts1 = 0;
    eCts2 = 0;
    eCts3 = 0;
    eCts4 = 0;
    eCts5 = 0;
    eCts6 = 0;
    eCts7 = 0;
    AB0 = 0;
    AB1 = 0;
    AB2 = 0;
    AB3 = 0;
    AB4 = 0;
    AB5 = 0;
    AB6 = 0;
    AB7 = 0;
//    'SET TIMER1 INTERRUPT to Timer1Clk and point Here:
//  
    
//'*************************************************************************************************
}
void loop()
{
  
//    IF PathOff THEN EXIT FUNCTION
//    if (PathOff) exit(0);
    if(runloop)
    {
//        if (PathOff) 
//        {
            //          'SET NEXT PATH CODE SECTION,  example only, needs much work
            //          '----------------------------------------------------------------------------------------------
//            pCycle++;  //'Increment current path's cycle position = 1 to MaxCycle
//            if (pCycle > PathCycles)
//            {
//                if (PathNext)
//                {
//                    pCycle = 0;
                    //'*************************************************************************************
                    //                'simulated paths: REMOVE AND REPLACE
                    pCts0 = -299 ; pCts1 = 197 ; pCts2 = 9   ; pCts3 = 23;   //   'motors
                    pCts4 = 10   ; pCts5 = -43 ; pCts6 = 200 ; pCts7 = -163; //   'image
                    //                'copy ABS values, set flag negative
                    pCtsA0 = abs(pCts0); pCtsA1 = abs(pCts1); pCtsA2 = abs(pCts2); pCtsA3 = abs(pCts3);
                    pCtsA4 = abs(pCts4); pCtsA5 = abs(pCts5); pCtsA6 = abs(pCts6); pCtsA7 = abs(pCts7);
                    //                'flag = 1 for negative path, 0 for positive
                    pCtsN0=bitRead(pCts0,15);pCtsN1=bitRead(pCts1,15);pCtsN2=bitRead(pCts2,15);pCtsN3=bitRead(pCts3,15);
                    pCtsN4=bitRead(pCts4,15);pCtsN5=bitRead(pCts5,15);pCtsN6=bitRead(pCts6,15);pCtsN7=bitRead(pCts7,15);
                    //'*************************************************************************************
//                    PathNext = 0;
//                }
//                else //no paths
//                {
//                    PathOff = 0;
                    //exit(0);
//                }
//            }
//        }
        //          '----------------------------------------------------------------------------------------------------
        //          'END OF SET NEXT PATH
        //          'Start of main interrupt code running at 60 * PathCycles. Example: 60Hz * 400 Pathcycles = 24KHz
        //          '----------------------------------------------------------------------------------------------------
        //          '(4) MOTOR CHANNELS:
        //        'skip altogether if every motor channel count equals zero
//        pCycle = 0;
        time1 = micros();
        //if (vLUT[pLUT[pCycle][pCtsA0]][pLUT[pCycle][pCtsA1]][pLUT[pCycle][pCtsA2]][pLUT[pCycle][pCtsA3]])
        for (pCycle = 1; pCycle< 1001; pCycle++)
        {
            
            if (1)
            {
                AB0 = eLUT[AB0][sLUT[pLUT[pCycle][pCtsA0]][pCtsN0]]; //'translate to AB code: = 0,1,2 or 3
                AB1 = eLUT[AB1][sLUT[pLUT[pCycle][pCtsA1]][pCtsN1]]; //'translate to AB code: = 0,1,2 or 3
                AB2 = eLUT[AB2][sLUT[pLUT[pCycle][pCtsA2]][pCtsN2]]; //'translate to AB code: = 0,1,2 or 3
                AB3 = eLUT[AB3][sLUT[pLUT[pCycle][pCtsA3]][pCtsN3]]; //'translate to AB code: = 0,1,2 or 3
                outportb(PORT1_DATA, bLUT[AB3][AB2][AB1][AB0]); //'write translated byte to motor port
    //            eCts0 += sLUT[pLUT[pCycle][pCtsA0]][pCtsN0];      //  'update motor counters
    //            eCts1 += sLUT[pLUT[pCycle][pCtsA1]][pCtsN1];
    //            eCts2 += sLUT[pLUT[pCycle][pCtsA2]][pCtsN2];
    //            eCts3 += sLUT[pLUT[pCycle][pCtsA3]][pCtsN3];
                
                
                eCts0 += pLUT[pCtsA0][pCycle];      //  'update motor counters
                eCts1 += pLUT[pCtsA1][pCycle];
                eCts2 += pLUT[pCtsA2][pCycle];
                eCts3 += pLUT[pCtsA3][pCycle];
               // eCts0 += pLUT[pCycle][pCtsA0];      //  'update motor counters
               // eCts1 += pLUT[pCycle][pCtsA1];
               // eCts2 += pLUT[pCycle][pCtsA2];
               // eCts3 += pLUT[pCycle][pCtsA3];
                
            }
            //          'skip altogether if every motor channel count equals zero
            //          '(4) IMAGE CHANNELS:
            if (1)
            //if (vLUT[pLUT[pCycle][pCtsA4]][pLUT[pCycle][pCtsA5]][pLUT[pCycle][pCtsA6]][pLUT[pCycle][pCtsA7]])
            {
                
                AB4 = eLUT[AB4][sLUT[pLUT[pCycle][pCtsA4]][pCtsN4]]; //'translate to AB code: = 0,1,2 or 3
                AB5 = eLUT[AB5][sLUT[pLUT[pCycle][pCtsA5]][pCtsN5]]; //'translate to AB code: = 0,1,2 or 3
                AB6 = eLUT[AB6][sLUT[pLUT[pCycle][pCtsA6]][pCtsN6]]; //'translate to AB code: = 0,1,2 or 3
                AB7 = eLUT[AB7][sLUT[pLUT[pCycle][pCtsA7]][pCtsN7]]; //'translate to AB code: = 0,1,2 or 3
                outportb(PORT2_DATA, bLUT[AB7][AB6][AB5][AB4]);  //'write translated byte to motor port
  //              eCts4 += sLUT[pLUT[pCycle][pCtsA4]][pCtsN4];      //  'update motor counters
  //              eCts5 += sLUT[pLUT[pCycle][pCtsA5]][pCtsN5];
  //              eCts6 += sLUT[pLUT[pCycle][pCtsA6]][pCtsN6];
  //              eCts7 += sLUT[pLUT[pCycle][pCtsA7]][pCtsN7];
  //              eCts4 += pLUT[pCycle][pCtsA4];      //  'update image counters
  //              eCts5 += pLUT[pCycle][pCtsA5];
  //              eCts6 += pLUT[pCycle][pCtsA6];
  //              eCts7 += pLUT[pCycle][pCtsA7];
                eCts4 += pLUT[pCtsA4][pCycle];      //  'update image counters
                eCts5 += pLUT[pCtsA5][pCycle];
                eCts6 += pLUT[pCtsA6][pCycle];
                eCts7 += pLUT[pCtsA7][pCycle];
 
            }
        }
        runloop = 0;
        time2 = micros();
     }
     
     microseconds = time2-time1;
//     Check_pLUT();
     delay(3000);
     
     Serial.print("Time in microseconds = : ");
     Serial.print(microseconds);
     Serial.println("");
     Serial.print("eCts0 = ");
     Serial.println(eCts0);
     Serial.print("eCts1 = ");
     Serial.println(eCts1);
     Serial.print("eCts2 = ");
     Serial.println(eCts2);
     Serial.print("eCts3 = ");
     Serial.println(eCts3);
     Serial.print("eCts4 = ");
     Serial.println(eCts4);
     Serial.print("eCts5 = ");
     Serial.println(eCts5);
     Serial.print("eCts6 = ");
     Serial.println(eCts6);
     Serial.print("eCts7 = ");
     Serial.println(eCts7);
     /*
     Serial.print("pCtsA0 = ");
     Serial.println(pCtsA0);
     Serial.print("pCtsA1 = ");
     Serial.println(pCtsA1);
     Serial.print("pCtsA2 = ");
     Serial.println(pCtsA2);
     Serial.print("pCtsA3 = ");
     Serial.println(pCtsA3);
     Serial.print("pCtsA4 = ");
     Serial.println(pCtsA4);
     Serial.print("pCtsA5 = ");
     Serial.println(pCtsA5);
     Serial.print("pCtsA6 = ");
     Serial.println(pCtsA6);
     Serial.print("pCtsA7 = ");
     Serial.println(pCtsA7);
   
     Serial.print("pCtsN0 = ");
     Serial.println(pCtsN0);
     Serial.print("pCtsN1 = ");
     Serial.println(pCtsN1);
     Serial.print("pCtsN2 = ");
     Serial.println(pCtsN2);
     Serial.print("pCtsN3 = ");
     Serial.println(pCtsN3);
     Serial.print("pCtsN4 = ");
     Serial.println(pCtsN4);
     Serial.print("pCtsN5 = ");
     Serial.println(pCtsN5);
     Serial.print("pCtsN6 = ");
     Serial.println(pCtsN6);
     Serial.print("pCtsN7 = ");
     Serial.println(pCtsN7);
     */
    //          '----------------------------------------------------------------------------------------------------
    //          'End of main interrupt code
}    

  
