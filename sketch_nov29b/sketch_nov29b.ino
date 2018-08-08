void setup() {
  // put your setup code here, to run once:

}

void loop() {
 
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
        printf("PathCnt: %ld %ld\n",PathCnt,Ctr);
        if (Ctr != PathCnt)
            ;//printf("Ctr != PathCnt\n");
        Ctr = 0;
    }
    
    printf("pLUT Check Completed\n");
}



}
