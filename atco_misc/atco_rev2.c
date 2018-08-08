//
//  main.c
//  atco_rev2
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
void Check_sLUT(void);
void Check_bLUT(void);
void Check_eLUT(void);
void Check_vLUT(void);
void Check_pLUT(void);
int main(int argc, const char * argv[]) {
    // insert code here...
    
    Check_bLUT();
    Check_sLUT();
    Check_eLUT();
    Check_pLUT();

    return 0;
}


//    '*******************************************************************************************
//    '  THIS CODE RUNS ONE TIME AT BEGINNING OF PROGRAM
//    '*******************************************************************************************

long  PathCycles, PathCnts, pCycle, pCnt;
double stp;
long Timer1Clk;
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
byte sLUT[2][2]  =  {1,2,1,0};
//    'eLUT: lookup table for setting encoder output code
byte eLUT[4][3] = {1,3,0,2,0,1,2,3,2,0,3,1};
//    'pLUT: path cycle lookup table
byte pLUT[401][401];
//    'bLUT: byte written to port LUT
byte bLUT[4][4][4][4] =
{
    0,64,128,192,16,80,144,208,32,96,160,224,48,112,176,240,4,68,132,196,20,84,148,212,
    36,100,164,228,52,116,180,244,8,72,136,200,24,88,152,216,40,104,168,232,56,120,184,248,
    12,76,140,204,28,92,156,220,44,108,172,236,60,124,188,252,1,65,129,193,17,81,145,209,
    33,97,161,225,49,113,177,241,5,69,133,197,21,85,149,213,37,101,165,229,53,117,181,245,
    9,73,137,201,25,89,153,217,41,105,169,233,57,121,185,249,13,77,141,205,29,93,157,221,
    45,109,173,237,61,125,189,253,2,66,130,194,18,82,146,210,34,98,162,226,50,114,178,242,
    6,70,134,198,22,86,150,214,38,102,166,230,54,118,182,246,10,74,138,202,26,90,154,218,
    42,106,170,234,58,122,186,250,14,78,142,206,30,94,158,222,46,110,174,238,62,126,190,254,
    3,67,131,195,19,83,147,211,35,99,163,227,51,115,179,243,7,71,135,199,23,87,151,215,
    39,103,167,231,55,119,183,247,11,75,139,203,27,91,155,219,43,107,171,235,59,123,187,251,
    15,79,143,207,31,95,159,223,47,111,175,239,63,127,191,255
};

void setup()
{
    
    Check_bLUT();
    TRUE = 1;
    PathCycles = 400;
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
    for (pCnt = PathCnts; pCnt > 1; pCnt--)
    {
        stp =(double)(PathCycles/pCnt);
        ctr = 0;
        do
        {
            pCycle = (long)(PathCycles - (stp*ctr));
            if (pCycle <1)
                break;
            else
                pLUT[pCycle][pCnt] = 1;
            ctr++;
        } while (1);
    }
    
    //    ' END OF STARTUP CODE
    //    '**************************************************************************************************
//    outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)outportb(PORT2_DIR, 0xff);
//    outportb(PORT1_DIR, 0xff); // set DIR register to 0xff (set all pins of GPIO port 0 are OUTPUT)
    //    'SET TIMER1 INTERRUPT to Timer1Clk and point Here:
    //    '*************************************************************************************************
}
void loop()
{
    //         IF PathOff THEN EXIT FUNCTION
    if (PathOff) exit(0);
    
    //          'SET NEXT PATH CODE SECTION,  example only, needs much work
    //          '----------------------------------------------------------------------------------------------
    pCycle++;  //'Increment current path's cycle position = 1 to MaxCycle
    if (pCycle > PathCycles)
    {
        if (PathNext)
        {
            pCycle = 0;
            //'*************************************************************************************
            //                'simulated paths: REMOVE AND REPLACE
            pCts0 = -299 ; pCts1 = 197 ; pCts2 = 9   ; pCts3 = 23;   //   'motors
            pCts4 = 10   ; pCts5 = -43 ; pCts6 = 200 ; pCts7 = -163; //   'image
            
            //                'copy ABS values, set flag negative
            pCtsA0 = abs(pCts0); pCtsA1 = abs(pCts1); pCtsA2 = abs(pCts2); pCtsA3 = abs(pCts3);
            pCtsA4 = abs(pCts4); pCtsA5 = abs(pCts5); pCtsA6 = abs(pCts6); pCtsA7 = abs(pCts7);
            
            //                'flag = 1 for negative path, 0 for positive
 //           pCtsN0=bitRead(pCts0,15);pCtsN1=bitRead(pCts1,15);pCtsN2=bitRead(pCts2,15);pCtsN3=bitRead(pCts3,15);
 //           pCtsN4=bitRead(pCts4,15);pCtsN5=bitRead(pCts5,15);pCtsN6=bitRead(pCts6,15);pCtsN7=bitRead(pCts7,15);
            //                '*************************************************************************************
        }
        else //no paths
        {
            PathOff = TRUE;
            exit(0);
        }
    }
    //          '----------------------------------------------------------------------------------------------------
    //          'END OF SET NEXT PATH
    //          'Start of main interrupt code running at 60 * PathCycles. Example: 60Hz * 400 Pathcycles = 24KHz
    //          '----------------------------------------------------------------------------------------------------
    //          '(4) MOTOR CHANNELS:
    //        'skip altogether if every motor channel count equals zero
    if (vLUT[pLUT[pCycle][pCtsA0]][pLUT[pCycle][pCtsA1]][pLUT[pCycle][pCtsA2]][pLUT[pCycle][pCtsA3]])
    {
        AB0 = eLUT[AB0][sLUT[pLUT[pCycle][pCtsA0]][pCtsN0]]; //'translate to AB code: = 0,1,2 or 3
        AB1 = eLUT[AB1][sLUT[pLUT[pCycle][pCtsA1]][pCtsN1]]; //'translate to AB code: = 0,1,2 or 3
        AB2 = eLUT[AB2][sLUT[pLUT[pCycle][pCtsA2]][pCtsN2]]; //'translate to AB code: = 0,1,2 or 3
        AB3 = eLUT[AB3][sLUT[pLUT[pCycle][pCtsA3]][pCtsN3]]; //'translate to AB code: = 0,1,2 or 3
 //       outportb(PORT1_DATA, bLUT[AB3][AB2][AB1][AB0]); //'write translated byte to motor port
        eCts0 += sLUT[pLUT[pCycle][pCtsA0]][pCtsN0];      //  'update motor counters
        eCts1 += sLUT[pLUT[pCycle][pCtsA1]][pCtsN1];
        eCts2 += sLUT[pLUT[pCycle][pCtsA2]][pCtsN2];
        eCts3 += sLUT[pLUT[pCycle][pCtsA3]][pCtsN3];
    }
    //          'skip altogether if every motor channel count equals zero
    //          '(4) IMAGE CHANNELS:
    if (vLUT[pLUT[pCycle][pCtsA4]][pLUT[pCycle][pCtsA5]][pLUT[pCycle][pCtsA6]][pLUT[pCycle][pCtsA7]])
    {
        
        AB4 = eLUT[AB4][sLUT[pLUT[pCycle][pCtsA4]][pCtsN4]]; //'translate to AB code: = 0,1,2 or 3
        AB5 = eLUT[AB5][sLUT[pLUT[pCycle][pCtsA5]][pCtsN5]]; //'translate to AB code: = 0,1,2 or 3
        AB6 = eLUT[AB6][sLUT[pLUT[pCycle][pCtsA6]][pCtsN6]]; //'translate to AB code: = 0,1,2 or 3
        AB7 = eLUT[AB7][sLUT[pLUT[pCycle][pCtsA7]][pCtsN7]]; //'translate to AB code: = 0,1,2 or 3
//        outportb(PORT1_DATA, bLUT[AB7][AB6][AB5][AB4]);  //'write translated byte to motor port
        eCts4 += sLUT[pLUT[pCycle][pCtsA4]][pCtsN4];      //  'update motor counters
        eCts5 += sLUT[pLUT[pCycle][pCtsA5]][pCtsN5];
        eCts6 += sLUT[pLUT[pCycle][pCtsA6]][pCtsN6];
        eCts7 += sLUT[pLUT[pCycle][pCtsA7]][pCtsN7];
    }
    //          '----------------------------------------------------------------------------------------------------
    //          'End of main interrupt code
}

//END FUNCTION



void Check_sLUT(void)
{
    long lCtr0, lCtr1;
    
    //'sLUT: -/+ count sign lookup table
    byte sLUT[2][2] = {1,1,2,0}; //'set count sign: -1=0,0=1,1=2
    
    //'TEST: tLUT: -/+ count sign lookup table
    byte tLUT[2][2]; //'set count sign: -1=0,0=1,1=2
    
    //'sLUT Generate:  count sign lookup tables
    tLUT[0][0] = 1; //'Count=0,Sign+ =0 / = 0 count
    tLUT[1][0] = 2; //'Count=1,Sign+ =0 / = +1 count
    tLUT[0][1] = 1; //'Count=0,Sign- =1 / = 0 count
    tLUT[1][1] = 0; //'Count=1,Sign- =1 / = -1 count
    
    for (lCtr1 = 0; lCtr1 < 2; lCtr1++)
    {
        for( lCtr0 = 0; lCtr0 < 2; lCtr0++)
        {
            if (tLUT[lCtr0][lCtr1] == sLUT[lCtr0][lCtr1])
                //'all OK
                ;
            else
               printf("error\n");
        }
    }
    printf("sLUT Check Completed\n");
    
}

void Check_bLUT()
{
    
    long lCtr0, lCtr1, lCtr2, lCtr3;
    byte eBit0, eBit1, eBit2, eBit3; //used for temp decoding encoder of AB LUT
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
   
    //'test: bLUT: byte written to port LUT
    byte tLUT[4][4][4][4];
    
    //'TEST: bLUT Generate:  port byte lookup table
    for (lCtr0 = 0; lCtr0 < 4; lCtr0++)
    {
        eBit0 = lCtr0;
        for (lCtr1 = 0; lCtr1 < 4; lCtr1++)
        {
            eBit1 = lCtr1 << 2;
            for (lCtr2 = 0; lCtr2 < 4; lCtr2++)
            {
                eBit2 = lCtr2 << 4;
                for ( lCtr3 = 0; lCtr3 < 4; lCtr3++)
                {
                    eBit3 = lCtr3 ; eBit3 = eBit3 << 6;
                    tLUT[lCtr3][lCtr2][lCtr1][lCtr0] = eBit3 | eBit2 | eBit1 | eBit0;
              //      printf("%i, ",bLUT[lCtr3][lCtr2][lCtr1][lCtr0]);
                }
               // printf("\n");
            }
            //printf("\n");
        }
        //printf("\n");
    }
    
    // 'TEST: bLUT Generate:  port byte lookup table
    for (lCtr0 = 0; lCtr0 < 4; lCtr0++)
    {
        for (lCtr1 = 0; lCtr1 < 4; lCtr1++)
        {
            for(lCtr2 = 0; lCtr2 < 4; lCtr2++)
            {
                for(lCtr3 = 0; lCtr3 < 4;lCtr3++)
                {
                    if (tLUT[lCtr3][lCtr2][lCtr1][lCtr0] == bLUT[lCtr3][lCtr2][lCtr1][lCtr0])
                        ;
                    else
                       printf("Error\n");
                    
                }
                //printf("\n");
            }
            //printf("\n");
        }
       // printf("\n");
    }
    
    printf("bLUT Check Completed\n");
}


void Check_eLUT(void)
{
    
    long lCtr0, lCtr1;
    
    //'eLUT: quadrature encoding lookup table
    byte eLUT[4][3] = {1,0,2,3,1,0,0,2,3,2,3,1}; //Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1
    //'tLUT: test quadrature encoding lookup table
    byte tLUT[4][3]; //'Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1
    //'TEST: tLUT Generate: encoding lookup tables
    //'Increment = -1       AB
    tLUT[0][0]= 1; //'AB OLD: 00 / AB NEW: 01
    tLUT[1][0]= 3; // 'AB OLD: 01 / AB NEW: 11
    tLUT[2][0]= 0; //'AB OLD: 10 / AB NEW: 00
    tLUT[3][0]= 2; //'AB OLD: 11 / AB NEW: 10
    //'Increment = 0         AB
    tLUT[0][1]= 0;  //'AB OLD: 00 / AB NEW: 00 = same, no change
    tLUT[1][1]= 1;  //'AB OLD: 01 / AB NEW: 01 = same, no change
    tLUT[2][1]= 2;  //'AB OLD: 10 / AB NEW: 10 = same, no change
    tLUT[3][1]= 3;  //'AB OLD: 11 / AB NEW: 11 = same, no change
    //'Increment = +1        AB
    tLUT[0][2]= 2; // 'AB OLD: 00 / AB NEW: 10
    tLUT[1][2]= 0; //'AB OLD: 01 / AB NEW: 00
    tLUT[2][2]= 3; //'AB OLD: 10 / AB NEW: 11
    tLUT[3][2]= 1; // 'AB OLD: 11 / AB NEW: 01
    
    for( lCtr1 = 0; lCtr1 < 4; lCtr1++)
    {
        for(lCtr0 = 0; lCtr0 < 3;lCtr0++)
        {
            if ( tLUT[lCtr0][lCtr1] == eLUT[lCtr0][lCtr1])
                ;
            else
            {
                printf("Error\n");
            }
            //printf("%d,", tLUT[lCtr1][lCtr0]);
            
        }
    }
    printf("eLUT Check Completed\n");
}

void Check_pLUT()
{
    double stpD, PathCyclesD, PathCycleD, PathCntsD, PathCntD, CtrD;
    
    long PathCycle, PathCycles, PathCnts,  PathCnt, Ctr;
    
    PathCnts = 2000;
    PathCycles = PathCnts; //'always equal: 1 cycle per Path count
    PathCyclesD = 2000 ;
    byte pLUT[2001][2001];
    
    //'******************************************************************************
    //'Create the Look up table  - one time only, could stored and loaded from disk!
    
    for(PathCnt = PathCnts; PathCnt > 0; PathCnt--)
    {
        PathCntD = PathCnt;
        stpD = (PathCyclesD/PathCntD);
        //printf("Stp = %f\n",stp);
        Ctr = 0;
        do
        {
            PathCycle = (PathCyclesD - (stpD * Ctr));
            if (PathCycle < 1)
                break;
            else
                pLUT[PathCnt][PathCycle] = 1;
            Ctr++;
        
        } while (1);
        
    }
    //'check results for errors
    Ctr=0;
    for (PathCnt = 1; PathCnt < PathCnts; PathCnt++)
    {
        for (PathCycle = 1; PathCycle < PathCycles; PathCycle++)
        {
            Ctr = Ctr + pLUT[PathCnt][PathCycle];
        }
        printf("PathCnt: %ld %ld\n",PathCnt,Ctr);
        if (Ctr != PathCnt)
            ;//printf("Ctr != PathCnt\n");
        Ctr = 0;
    }
    
    printf("pLUT Check Completed\n");
}

void Check_vLUT()
{
    //'vLUT: flag if value of any path equals 1
    byte vLUT[2][2][2][2] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    long lCtr0, lCtr1, lCtr2, lCtr3;
    
    goto skip1;
    
    //'vLUT Generate: set flag if value of any path equals 1
    for( lCtr0 = 0; lCtr0 < 2; lCtr0++)
    {
        for( lCtr1 = 0; lCtr1 < 2; lCtr1++)
        {
            for( lCtr2 = 0; lCtr2 < 2; lCtr2++)
            {
                for( lCtr3 = 0; lCtr3 < 2; lCtr3++)
                {
                    if (lCtr0 | lCtr1 | lCtr2 | lCtr3)
                        vLUT[lCtr0][lCtr1][lCtr2][lCtr3] = 1;
                    else
                        vLUT[lCtr0][lCtr1][lCtr2][lCtr3] = 0;
                }
            }
        }
    }
    
skip1:
    //'vLUT Generate: set flag if value of any path equals 1
    for( lCtr0 = 0; lCtr0 < 2; lCtr0++)
    {
        for( lCtr1 = 0; lCtr1 < 2; lCtr1++)
        {
            for( lCtr2 = 0; lCtr2 < 2; lCtr2++)
            {
                for( lCtr3 = 0; lCtr3 < 2; lCtr3++)
                {
                    if (lCtr0 | lCtr1 | lCtr2 | lCtr3)
                    {
                        if (vLUT[lCtr0][lCtr1][lCtr2][lCtr3])
                            printf("ok\n");
                        else
                        {
                            printf("error\n");
                        }
                        
                    }
                    else
                    {
                        if (vLUT[lCtr0][lCtr1][lCtr2][lCtr3] == 0)
                            printf("ok\n");
                        else
                        {
                            printf("error\n");
                           
                        }
                    }
                }
            }
        }
    }
    
    printf( "vLUT Check Completed\n");
}
