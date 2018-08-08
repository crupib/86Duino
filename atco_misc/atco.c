//'*******************************************************************************************
//' THIS CODE RUNS ONE TIME AT BEGINNING OF PROGRAM
//'*******************************************************************************************

double PathCycles, Cycle, MaxPathCnts, Cnt, stp;
long Timer1Clk, PathOff, PathCycle, PathNext AS LONG

long lctr, lctr0, lctr1, lctr2, lctr3 ; //'loop counters
byte eBit0, ebit1, ebit2, ebit3; //'used for temp decoding encoder of AB LUT
byte eByte1, eByte2;/'port write byte
int MaxPath;

//'Path counts are spread out into the fixed number of path cycles.
//'The path count must not exceed the fixed number of path cycles
//'Set path cycles are ran continuous irrelevent of path counts
PathCycles = 400;

//'Maximum path counts equals the allowed limit any one path count can be.
//'examples, Path Counts = 400; 400/400 = 1 count output per 1 cycle
//' Path Counts = 200; 400/200 = 1 count output per 2 cycles
//' Path Counts = 150; 400/150 = 1 count output per 2 or 3 cycles
//' Path Counts = 1; 400/1 = 1 count output per 400 cycles
MaxPathCnts = PathCycles;

Timer1Clk = 60 * PathCycles; // 'Timer1 clock = 60Hz * 400 Pathcycles = 24KHz

byte AB[8];   //'(8) quad encoder outputs, (4) motor and (4) image
long eCts[8]; //'(8) encoder output counters, (4) motor and (4) image

int eInc[8];  //'(8) increment buffers, (4) motor and (4) image
int pCts[8];  //'current raw path count
int pCtsA[8]; //'current abs path count
int pCtsN[8]; //'negative path count flag, equals True or False

//'LUT: path cycle lookup table
byte pLUT[PathCycles+1][MaxPathCnts+1]];

//'LUT: -/+ count sign lookup table
byte sLUT[2][2]; //'set count sign: -1=0,0=1,1=2

//'LUT: quadrature encoding lookup table
byte eLUT[4][3]; //'Current Encoder Val: 0-3, Encoder Count Inc: -1, 0, 1

//'LUT: write byte to port LUT
byte bLUT[4][4][4][4];

//'LUT Generate: port byte lookup tables
for(lctr0 = 0; lctr0 < 4; lctr0++)
{ 
	ebit0 = lctr0;
	for (lctr1 = 0; lctr1 < 4; lctr1++)
	{	
		ebit1 = lctr1;
		ebit1 = ebit1 << 2;
		for (lctr2 = 0; lctr2 < 4; lctr2++)
		{
			ebit2 = lctr2;
			ebit2 = ebit2 << 4;
			for (lctr3 = 0; lctr3 < 4; lctr3++)
			{	
				ebit3 = lCtr3;
				ebit3 = ebit3 << 6
				bLUT[lctr3,lctr2,lctr1,lctr0] = ebit3 | ebit2 | ebit1 | ebit0;
			}
		}
	}
}

//'LUT Generate: count sign lookup tables
sLUT[0][0] = 1; //'Count=0,Sign+ =0 / = 0 count
sLUT[0][1) = 1; //'Count=0,Sign- =1 / = 0 count
sLUT[1][0]=  2; //'Count=1,Sign+ =0 / = +1 count
sLUT[1][1]=  0; //'Count=1,Sign- =1 / = -1 count


//'LUT Generate: encoding lookup tables
//'Increment = -1 AB
eLUT[0][0]= 1; //'AB OLD: 00 / AB NEW: 01
eLUT[1][0]= 3; //'AB OLD: 01 / AB NEW: 11
eLUT[2][0]= 0; //'AB OLD: 10 / AB NEW: 00
eLUT[3][0]= 2; //'AB OLD: 11 / AB NEW: 10
//'Increment = 0 AB
eLUT[0][1]= 0; //'AB OLD: 00 / AB NEW: 00 = same, no change
eLUT[1][1)= 1; //'AB OLD: 01 / AB NEW: 01 = same, no change
eLUT[2][1)= 2; //'AB OLD: 10 / AB NEW: 10 = same, no change
eLUT[3][1)= 3; //'AB OLD: 11 / AB NEW: 11 = same, no change
//'Increment = +1 AB
eLUT[0][2)= 2; //'AB OLD: 00 / AB NEW: 10
eLUT[1][2)= 0; //'AB OLD: 01 / AB NEW: 00
eLUT[2][2)= 3; //'AB OLD: 10 / AB NEW: 11
eLUT[3][2)= 1; //'AB OLD: 11 / AB NEW: 01


//'LUT Generate: Path cycle lookup table - one time only, could be stored and loaded from disk!
for(Cycle = 1; Cycle <= PathCycles; Cycle++)
{
	stp = -(PathCycles/Cycle);
	for (Cnt = PathCycles; PathCycles > 1; Cnt+=stp)
	{
		pLUT[Cycle][Cnt] = 1; //'set count flag to 1
	}
}


//' END OF STARTUP CODE
//'**************************************************************************************************
//'SET TIMER1 INTERRUPT to Timer1Clk and point Here:
//'*************************************************************************************************

if (PathOff)  
	exit;

//'SET NEXT PATH CODE SECTION, example only, needs much work
'----------------------------------------------------------------------------------------------
PathCycle++;                //'Increment current path's cycle position = 1 to MaxCycle
if (PathCycle > PathCycles) //'all path cycles complete,fetch next path if any.
	if (PathNext)           //'path in buffer?
		PathCycle = 0;	     //'clear cycle
//'NEXT PATH: PREPARE THIS OUTSIDE OF INTERRUPT FUNCTION?

//'*************************************************************************************
//'simulated paths: REMOVE AND REPLACE
pCts[0] = -299;
pCts[1] = 197;
pCts[2] = 9;
pCts[3] = 23; // 'motors
pCts[4] = 10; 
pCts[5] = -43; 
pCts[6] = 200; 
pCts[7] = -163; //'image

//'copy ABS values, set flag negative
FOR (lctr = 0; lctr < 8; lctr++)
{
	pCtsA(lctr) = ABS(pCts(lctr)) 'get ABS value of each path
	pCtsN(lctr) = BIT(pCts(lctr),15) 'flag = 1 for negative path, 0 for positive
}
//'*************************************************************************************

else //'no paths
{
	PathOff = TRUE ;
	exit();
}

}

}
//'-------------------------------------------------------------------------------------------
//'END OF SET NEXT PATH


//'Start of main interrupt code running at 60 * PathCycles. Example: 60Hz * 400 Pathcycles = 24KHz
//'code below uses lookup tables, no divide's or multiply's, just memory addressing.
//'-------------------------------------------------------------------------------------------------------------------

//'(4) MOTOR CHANNELS:

//'skip altogether if every motor channel count equals zero
if pLUT[PathCycle][pCtsA[0] | pLUT[PathCycle][pCtsA[1]] | pLUT[PathCycle][pCtsA[2]] | pLUT[PathCycle][pCtsA[3]] 
{}

FOR lctr = 0 TO 3
eInc(lctr)= pLUT(PathCycle,pCtsA(lctr)) 'translate current count from path_LUT
eInc(lctr)= sLUT(eInc(lctr),pCtsN(lctr)) 'translate to +/- from sign_LUT: -1=0,0=1,1=2
AB(lctr)= eLUT(AB(lctr),eInc(lctr)) 'translate to AB code from encoder_LUT
eCts(lCtr)+= eInc(lctr) 'update motor counters
NEXT

OUT PORT(port1,bLUT(AB(3),AB(2),AB(1),AB(0))) 'write translated byte to motor port

}

//'(4) IMAGE CHANNELS:

'skip altogether if every image channel count equals zero
IF pLUT(PathCycle,pCtsA(4)) OR pLUT(PathCycle,pCtsA(5)) OR pLUT(PathCycle,pCtsA(6)) OR pLUT(PathCycle,pCtsA(7)) 
{


	FOR lctr = 4 TO 7
	{
		eInc(lctr)= pLUT(PathCycle,pCtsA(lctr)) 'translate current count from path_LUT
		eInc(lctr)= sLUT(eInc(lctr),pCtsN(lctr)) 'translate to +/- from sign_LUT: -1=0,0=1,1=2
		AB(lctr)= eLUT(AB(lctr),eInc(lctr)) 'translate to AB code from encoder_LUT
		eCts(lCtr)+= eInc(lctr) 'update Image counters
	}

	OUT PORT(port2,bLUT(AB(7),AB(6),AB(5),AB(4))) 'write translated byte to image port

}

//'-------------------------------------------------------------------------------------------------------
//'End of main interrupt code

 