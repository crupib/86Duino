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

//    '*******************************************************************************************
//    '  THIS CODE RUNS ONE TIME AT BEGINNING OF PROGRAM
//    '*******************************************************************************************

    long PathCycles, PathCnts, pCycle, pCnt;
    double stp;
    long Timer1Clk; 
    long ctr, lctr; //loop counters
    long PathOff, PathNext;
    long TRUE;
    byte AB0,AB1,AB2,AB3,AB4,AB5,AB6,AB7;                        //'quad coded outputs  
    long eCts0,eCts1,eCts2,eCts3,eCts4,eCts5,eCts6,eCts7;        //'encoder counters
    int eInc0,eInc1,eInc2,eInc3,eInc4,eInc5,eInc6,eInc7;         //'path increment
    int pCts0,pCts1,pCts2,pCts3,pCts4,pCts5,pCts6,pCts7;         //'raw path count
    int pCtsA0,pCtsA1,pCtsA2,pCtsA3,pCtsA4,pCtsA5,pCtsA6,pCtsA7; //'abs path count
    int pCtsN0,pCtsN1,pCtsN2,pCtsN3,pCtsN4,pCtsN5,pCtsN6,pCtsN7; //'-path flag
    //    'vLUT: flag if value of any path equals 1
    byte vLUT[2][2][2][2];
    PathCycles = 400;
    PathCnts = PathCycles; //'always equals PathCycles

//    'pLUT: path cycle lookup table
    byte pLUT[PathCycles+1] [PathCnts+1];
//    'sLUT: lookup table to set count to + or - sign
    byte sLUT[2]; //'set count sign: -1=0,0=1,1=2
//    'eLUT: quadrature encoding lookup table
    byte eLUT[4][3]; //'Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1
//    'bLUT: byte written to port LUT
    byte bLUT[4][4][4][4];

    void setup()
    {
      Serial.begin(9600);
      TRUE = 1;

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

//    '**************************************************************************************
//    ' ASSIGN VALUES TO ALL LOOKUP TABLES
//    '**************************************************************************************
//    'bLUT: lookup table for byte written to 8 bit encode port

      bLUT[][][][] 
      = {
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
//    'vLUT: lookup table, TRUE if value of any path inc equals 1
      vLUT[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//    'sLUT: lookup table to set count to + or - sign
      sLUT[] = [] = {1,2,1,0};
//    'eLUT: lookup table for setting encoder output code
      eLUT[][] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
//    'pLUT: generate lookup table, based on Path cycle and Path counts
//    ' all varibles are longs except stp which is a float, double
    for (pCnt = PathCnts; pCnt > 1; pCnt--)
    {
        stp =(double)Pathcycles/pCnt;
        ctr = 0;
        do
        {
          pCycle = (long)(Pathcycles - (stp*ctr));
          if (pCycle <1)
            break;
          else
            pLUT[pCycle][pCnt] = 1;
          ctr++;
        } while (1);
    }
}
//    ' END OF STARTUP CODE
//    '**************************************************************************************************
    outportb(PORT2_DIR, 0xff); // set DIR register to 0xff (eByte2 clear)outportb(PORT2_DIR, 0xff); 
    outportb(PORT1_DIR, 0xff); // set DIR register to 0xff (set all pins of GPIO port 0 are OUTPUT)
//    'SET TIMER1 INTERRUPT to Timer1Clk and point Here:
//    '*************************************************************************************************
    loop
    {
//         IF PathOff THEN EXIT FUNCTION
            if (PathOff) exit();

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
                pCtsA0 = ABS[pCts0]; pCtsA1 = ABS[pCts1];pCtsA2 = ABS[pCts2] ; pCtsA3 = ABS[pCts3];
                pCtsA4 = ABS[pCts4]; pCtsA5 = ABS[pCts5]; pCtsA6 = ABS[pCts6];pCtsA7 = ABS[pCts7];

//                'flag = 1 for negative path, 0 for positive
                pCtsN0=bitRead(pCts0,15);pCtsN1=bitRead(pCts1,15);pCtsN2=bitRead(pCts2,15);pCtsN3=bitRead(pCts3,15);
                pCtsN4=bitRead(pCts4,15);pCtsN5=bitRead(pCts5,15);pCtsN6=bitRead(pCts6,15);pCtsN7=bitRead(pCts7,15);
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
          if vLUT[pLUT[pCycle][pCtsA0]][pLUT[pCycle][pCtsA1]][pLUT[pCycle][pCtsA2]][pLUT[pCycle][pCtsA3]] 
          { 
             AB0 = eLUT[AB0][sLUT[pLUT[pCycle][pCtsA0][pCtsN0]]; //'translate to AB code: = 0,1,2 or 3
             AB1 = eLUT[AB1][sLUT[pLUT[pCycle][pCtsA1][pCtsN1]]; //'translate to AB code: = 0,1,2 or 3
             AB2 = eLUT[AB2][sLUT[pLUT[pCycle][pCtsA2][pCtsN2]]; //'translate to AB code: = 0,1,2 or 3
             AB3 = eLUT[AB3][sLUT[pLUT[pCycle][pCtsA3][pCtsN3]]; //'translate to AB code: = 0,1,2 or 3
             outportb(PORT1_DATA, bLUT[AB3][AB2][AB1][AB0]); //'write translated byte to motor port         
             eCts0 += sLUT[pLUT[pCycle][pCtsA0][pCtsN0]];      //  'update motor counters
             eCts1 += sLUT[pLUT[pCycle][pCtsA1][pCtsN1]]; 
             eCts2 += sLUT[pLUT[pCycle][pCtsA2][pCtsN2]]; 
             eCts3 += sLUT[pLUT[pCycle][pCtsA3][pCtsN3]]; 
          }
//          'skip altogether if every motor channel count equals zero       
//          '(4) IMAGE CHANNELS:
          if vLUT[pLUT[pCycle][pCtsA4]][pLUT[pCycle][pCtsA5]][pLUT[pCycle][pCtsA6]][pLUT[pCycle][pCtsA7]] 
          { 
             
             AB4 = eLUT[AB4][sLUT[pLUT[pCycle][pCtsA4][pCtsN4]]; //'translate to AB code: = 0,1,2 or 3
             AB5 = eLUT[AB5][sLUT[pLUT[pCycle][pCtsA5][pCtsN5]]; //'translate to AB code: = 0,1,2 or 3
             AB6 = eLUT[AB6][sLUT[pLUT[pCycle][pCtsA6][pCtsN6]]; //'translate to AB code: = 0,1,2 or 3
             AB7 = eLUT[AB7][sLUT[pLUT[pCycle][pCtsA7][pCtsN7]]; //'translate to AB code: = 0,1,2 or 3
             outportb(PORT1_DATA, bLUT[AB7][AB6][AB5][AB4]);  //'write translated byte to motor port   
             eCts4 += sLUT[pLUT[pCycle][pCtsA4][pCtsN4]];      //  'update motor counters
             eCts5 += sLUT[pLUT[pCycle][pCtsA5][pCtsN5]]; 
             eCts6 += sLUT[pLUT[pCycle][pCtsA6][pCtsN6]]; 
             eCts7 += sLUT[pLUT[pCycle][pCtsA7][pCtsN7]]; 
          }
//          '----------------------------------------------------------------------------------------------------
//          'End of main interrupt code
}

//END FUNCTION



SUB Check_sLUT

    LOCAL lctr0, lctr1 AS LONG

    'sLUT: -/+ count sign lookup table
    DIM sLUT(1,1) AS LOCAL BYTE 'set count sign: -1=0,0=1,1=2

    'TEST: tLUT: -/+ count sign lookup table
    DIM tLUT(1,1) AS LOCAL BYTE 'set count sign: -1=0,0=1,1=2

    'sLUT Generate:  count sign lookup tables
    tLUT(0,0)= 1 'Count=0,Sign+ =0 / = 0 count
    tLUT(1,0)= 2 'Count=1,Sign+ =0 / = +1 count
    tLUT(0,1)= 1 'Count=0,Sign- =1 / = 0 count
    tLUT(1,1)= 0 'Count=1,Sign- =1 / = -1 count

    ARRAY ASSIGN sLUT() = 1,2,1,0

    FOR lCtr1 = 0 TO 1
        FOR lCtr0 = 0 TO 1
            IF tLUT(lCtr0,lCtr1) = sLUT(lCtr0,lCtr1) THEN
               'all OK
            ELSE
               PRINT "Error"
            END IF
        NEXT
    NEXT

    PRINT "sLUT Check Completed"
    WAITKEY$



END SUB

SUB Check_bLUT

    LOCAL lctr0, lctr1, lctr2, lctr3 AS LONG


    LOCAL eBit0, ebit1, ebit2, ebit3 AS BYTE 'used for temp decoding encoder of AB LUT


    'bLUT: byte written to port LUT
    DIM bLUT(3,3,3,3) AS LOCAL BYTE

    'test: bLUT: byte written to port LUT
    DIM tLUT(3,3,3,3) AS LOCAL BYTE


    'TEST: bLUT Generate:  port byte lookup table
    FOR lctr0 = 0 TO 3
        ebit0 = lctr0
        FOR lctr1 = 0 TO 3
            ebit1 = lctr1 : SHIFT LEFT ebit1,2
            FOR lctr2 = 0 TO 3
                ebit2 = lctr2 : SHIFT LEFT ebit2,4
                FOR lctr3 = 0 TO 3
                    ebit3 = lCtr3 : SHIFT LEFT ebit3,6
                    tLUT(lctr3,lctr2,lctr1,lctr0) = ebit3 OR ebit2 OR ebit1 OR ebit0
                NEXT
            NEXT
        NEXT
    NEXT


    ARRAY ASSIGN bLUT() = _
    0,64,128,192,16,80,144,208,32,96,160,224,48,112,176,240,4,68,132,196,20,84,148,212,_
    36,100,164,228,52,116,180,244,8,72,136,200,24,88,152,216,40,104,168,232,56,120,184,248,_
    12,76,140,204,28,92,156,220,44,108,172,236,60,124,188,252,1,65,129,193,17,81,145,209,_
    33,97,161,225,49,113,177,241,5,69,133,197,21,85,149,213,37,101,165,229,53,117,181,245,_
    9,73,137,201,25,89,153,217,41,105,169,233,57,121,185,249,13,77,141,205,29,93,157,221,_
    45,109,173,237,61,125,189,253,2,66,130,194,18,82,146,210,34,98,162,226,50,114,178,242,_
    6,70,134,198,22,86,150,214,38,102,166,230,54,118,182,246,10,74,138,202,26,90,154,218,_
    42,106,170,234,58,122,186,250,14,78,142,206,30,94,158,222,46,110,174,238,62,126,190,254,_
    3,67,131,195,19,83,147,211,35,99,163,227,51,115,179,243,7,71,135,199,23,87,151,215,_
    39,103,167,231,55,119,183,247,11,75,139,203,27,91,155,219,43,107,171,235,59,123,187,251,_
    15,79,143,207,31,95,159,223,47,111,175,239,63,127,191,255



     'TEST: bLUT Generate:  port byte lookup table
    FOR lctr0 = 0 TO 3
        FOR lctr1 = 0 TO 3
            FOR lctr2 = 0 TO 3
                FOR lctr3 = 0 TO 3
                    IF tLUT(lctr3,lctr2,lctr1,lctr0) = bLUT(lctr3,lctr2,lctr1,lctr0) THEN
                       'all OK
                    ELSE
                        PRINT "Error"
                    END IF
                NEXT
            NEXT
        NEXT
    NEXT

    PRINT "bLUT Check Completed"
    WAITKEY$



END SUB


SUB Check_eLUT

     LOCAL lctr0, lctr1 AS LONG

    'eLUT: quadrature encoding lookup table
    DIM eLUT(3,2) AS LOCAL BYTE 'Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1

    'tLUT: test quadrature encoding lookup table
    DIM tLUT(3,2) AS LOCAL BYTE 'Current Encoder Val: 0-3,   Encoder Count Inc: -1, 0, 1



    'TEST: tLUT Generate: encoding lookup tables
    'Increment = -1       AB
    tLUT(0,0)= 1 'AB OLD: 00 / AB NEW: 01
    tLUT(1,0)= 3 'AB OLD: 01 / AB NEW: 11
    tLUT(2,0)= 0 'AB OLD: 10 / AB NEW: 00
    tLUT(3,0)= 2 'AB OLD: 11 / AB NEW: 10
    'Increment = 0         AB
    tLUT(0,1)= 0  'AB OLD: 00 / AB NEW: 00 = same, no change
    tLUT(1,1)= 1  'AB OLD: 01 / AB NEW: 01 = same, no change
    tLUT(2,1)= 2  'AB OLD: 10 / AB NEW: 10 = same, no change
    tLUT(3,1)= 3  'AB OLD: 11 / AB NEW: 11 = same, no change
    'Increment = +1        AB
    tLUT(0,2)= 2  'AB OLD: 00 / AB NEW: 10
    tLUT(1,2)= 0  'AB OLD: 01 / AB NEW: 00
    tLUT(2,2)= 3  'AB OLD: 10 / AB NEW: 11
    tLUT(3,2)= 1  'AB OLD: 11 / AB NEW: 01



    ARRAY ASSIGN eLUT() = 1,3,0,2,0,1,2,3,2,0,3,1


    FOR lCtr1 = 0 TO 2
        FOR lCtr0 = 0 TO 3
            IF tLUT(lCtr0,lCtr1) = eLUT(lCtr0,lCtr1) THEN
               'all OK
            ELSE
               PRINT "Error"
            END IF
        NEXT
    NEXT



    PRINT "eLUT Check Completed"
    WAITKEY$


END SUB


SUB Check_pLUT



    LOCAL stp AS DOUBLE

    LOCAL PathCycle, PathCycles, PathCnts,  PathCnt, Ctr AS LONG 'DOUBLE

    PathCnts = 2000 : PathCycles = PathCnts 'always equal: 1 cycle per Path count

    DIM pLUT(PathCycles,PathCnts) AS LOCAL BYTE

    '******************************************************************************
    'Create the Look up table  - one time only, could stored and loaded from disk!

    FOR PathCnt = PathCnts TO 1 STEP -1
        stp = CDBL(PathCycles/PathCnt)
        ctr = 0
        DO

           PathCycle = CLNG(PathCycles - (stp * ctr))
           IF PathCycle < 1 THEN
              EXIT DO
           ELSE
              pLUT(PathCycle,PathCnt) = 1
           END IF
           INCR ctr

        LOOP
    NEXT

    'check results for errors
    ctr=0
    FOR PathCnt = 1 TO PathCnts
        FOR PathCycle = 1 TO PathCycles
            Ctr = Ctr + pLUT(PathCycle,PathCnt)
        NEXT
        PRINT "PathCnt: "; PathCnt;" Ctr: ";Ctr
        IF Ctr <> PathCnt THEN WAITKEY$
        Ctr = 0
    NEXT

    PRINT "DONE !"
    WAITKEY$
    EXIT SUB


END SUB


SUB Check_vLUT

    'vLUT: flag if value of any path equals 1
    DIM vLUT(1,1,1,1) AS LOCAL BYTE

    LOCAL lctr0, lctr1, lctr2, lctr3 AS LONG

    ARRAY ASSIGN vLUT() = 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1

    GOTO skip1

    'vLUT Generate: set flag if value of any path equals 1
    FOR lctr0 = 0 TO 1
        FOR lctr1 = 0 TO 1
            FOR lctr2 = 0 TO 1
                 FOR lctr3 = 0 TO 1
                     IF lctr0 OR lctr1 OR lctr2 OR lctr3 THEN
                        vLUT(lctr0,lctr1,lctr2,lctr3) = 1
                     ELSE
                        vLUT(lctr0,lctr1,lctr2,lctr3) = 0
                     END IF
                 NEXT
            NEXT
        NEXT
    NEXT


skip1:
     'vLUT Generate: set flag if value of any path equals 1
    FOR lctr0 = 0 TO 1
        FOR lctr1 = 0 TO 1
            FOR lctr2 = 0 TO 1
                 FOR lctr3 = 0 TO 1
                     IF lctr0 OR lctr1 OR lctr2 OR lctr3 THEN
                        IF vLUT(lctr0,lctr1,lctr2,lctr3) THEN
                           'all ok
                        ELSE
                           PRINT "error"
                           WAITKEY$
                        END IF
                     ELSE
                        IF vLUT(lctr0,lctr1,lctr2,lctr3) = 0 THEN
                           'all ok
                        ELSE
                           PRINT "error"
                           WAITKEY$
                        END IF
                     END IF
                 NEXT
            NEXT
        NEXT
    NEXT

    PRINT "vLUT Check Completed"
    WAITKEY$


END SUB
