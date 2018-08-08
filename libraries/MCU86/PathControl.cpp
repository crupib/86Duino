
#include "Arduino.h" 
#include "MCU86.h"
#include <Encoder.h>
#include <TimerOne.h>


// test variable
extern uint32_t test1;

// external variables
extern uint8_t status_byte;         // status byte
extern uint8_t enc_error_flg;        // encoder error flag
extern uint16_t  enc_error_band1,enc_error_band2,enc_error_band3; 
extern uint8_t step_rate;            // step rate multiplier


// local variables
int16_t PathBuffer1[BUFFER_SIZE];   // Position path buffer 1
int16_t PathBuffer2[BUFFER_SIZE];   // Position path buffer 2
int16_t PathBuffer3[BUFFER_SIZE];   // Position path buffer 3
int16_t PathBuffer4[BUFFER_SIZE];   // Position path buffer 4
int16_t PathBuffer5[BUFFER_SIZE];   // Position path buffer 5

int16_t  PathIn[5],PathOut[5];      // input/output path position

int16_t PointerIn,PointerOut;       // Buffer path input/outputpointer
int16_t DataInPathBuf;              // number of paths in buffer
int16_t SpaceAvailable;             // buffer space available

uint32_t PathsExecuted;           // total number of paths executed
uint32_t PathsPlusBuf;            // total number of paths plus buffer
uint32_t PathTimer;               // path timer variable
uint32_t STimer;                  // smoothing timer

uint8_t path_error;                // path buffer error
uint8_t image_count_dis;          // disable / enable counts to image system

int32_t scount1, scount2, scount3, scount4, scount5; // internal counters
uint32_t period1, period2, period3, period4, period5;  // smoothong period
int16_t PCount1, PCount2, PCount3, PCount4, PCount5; // path counters
uint8_t Dir1, Dir2, Dir3, Dir4, Dir5;    // Path direction
uint32_t Stime1,Stime2,Stime3,Stime4,Stime5; // start time

uint8_t last_path_flg;       // last path flag
uint8_t motion;              // motion mode
uint8_t jog_mode;            // jog mode
uint8_t remote_mode;          // remote mode


//*********************************************************
// Path buffer control
// Initialize Path buffer to reset
//**********************************************************

void InitPathBuf() {

//    StopMotion();                 // Stop motor motion
    
    PointerOut=0;                   // initialize output pointer
    PointerIn=0;                    // initialize input pointer
    DataInPathBuf=0;                // number of paths in buffer
    SpaceAvailable=BUFFER_SIZE;     // buffer space available 
    STimer=SPATH_TIMER;             // 60Hz smooth timer, adds hold time
    PathTimer=PATH_TIMER;           // 60Hz path timer 
    
    PathsExecuted=0;                // total number of paths executed
    PathsPlusBuf=0;                 // total number of paths executed plus paths in buffer

    scount1=0;                      // reset internal counters
    scount2=0;
    scount3=0;
    scount4=0;
    scount5=0;

    enc_error_band1=0;                // encoder error band
    enc_error_band2=0;
    enc_error_band3=0;
}

//**********************************************************
//Write data to all 5 path encoder buffers
//**********************************************************
void WritePathData(){

    ++PointerIn;                // increment data in pointer

    if(PointerIn==BUFFER_SIZE-1)
        PointerIn=0;            // roll over pointer

    ++DataInPathBuf;            // increment path counter   
    if(PointerIn==PointerOut)
        path_error=1;
    else {
        (PathBuffer1[PointerIn])=PathIn[0];
        (PathBuffer2[PointerIn])=PathIn[1];
        (PathBuffer3[PointerIn])=PathIn[2];
        (PathBuffer4[PointerIn])=PathIn[3];
        (PathBuffer5[PointerIn])=PathIn[4];
        path_error=0;
    }
}    

//**********************************************************    
//Read path data for all 5 encoder output buffers   
void ReadPathData() {
  
    if(DataInPathBuf==0)
        status_byte|=0x02;              //  Tried to read from empty Buffer, data lost
    else {
        ++PointerOut;                   // increment data out pointer
        --DataInPathBuf;                // decrement path counter

        if(PointerOut==BUFFER_SIZE-1)
            PointerOut=0;               // roll over pointer
        
        PathOut[0]=(PathBuffer1[PointerOut]);
        PathOut[1]=(PathBuffer2[PointerOut]);
        PathOut[2]=(PathBuffer3[PointerOut]);
        PathOut[3]=(PathBuffer4[PointerOut]);
        PathOut[4]=(PathBuffer5[PointerOut]);
                
    }
}

//**********************************************************
// return buffer space available
// 0 = path buffer is empty
//**********************************************************
int16_t PathSpaceAvail() {
    return(BUFFER_SIZE-DataInPathBuf);
}


//**********************************************************
// Start new path position
void NewPosition() {
  
    EncoderMotionError();                  // check for encoder position error
     
    if(DataInPathBuf) {                    // check for data in buffer            

        if(DataInPathBuf==1) {              // last path position
            last_path_flg=1;

            if(jog_mode==OFF) {             // if Jog Mode OFF            
              Timer1.detachInterrupt();     // turn off interrupt
              motion=OFF;
            }

        } //end if


// Get next path position
        ReadPathData();                         // get next path positions       
        PathsExecuted++;                        // increment total number of paths executed
                        
        PCount1=PathOut[0];                    // save individual path counts                     
        PCount2=PathOut[1];
        PCount3=PathOut[2];
        PCount4=PathOut[3];
        PCount5=PathOut[4];                                                                        

               
// calculate count period and direction 
// quad 1
            if(PCount1<0) Dir1=NEG;                 // set direction negative
            else Dir1=POS;                          // set direction positive

            PCount1=abs(PCount1);                   // Make Path position positive        
            if(PCount1!=0) period1=STimer/PCount1;  // calculate time spacing between positions       
            Stime1=micros();                        // start time 1
                            
// quad 2                                
            if(PCount2<0) Dir2=NEG;                 // set direction negative
            else Dir2=POS;                          // set direction positive

            PCount2=abs(PCount2);                   // Make Path position positive       
            if(PCount2!=0) period2=STimer/PCount2;   // calculate time spacing between positions      
            Stime2=micros();                        // start time 
            
// quad 3                                     
            if(PCount3<0) Dir3=NEG;                     // set direction negative
            else Dir3=POS;                              // set direction positive

            PCount3=abs(PCount3);                   // Make Path position positive        
            if(PCount3!=0) period3=STimer/PCount3;   // calculate time spacing between positions       
            Stime3=micros();                        // start time 

// quad 4     
            if(PCount4<0) Dir4=NEG;                 // set direction negative
            else  Dir4=POS;                         // set direction positive

            PCount4=abs(PCount4);                   // Make Path position positive        
            if(PCount4!=0) period4=STimer/PCount4;   // calculate time spacing between positions        
            Stime4=micros();                        // start time 
            
// quad 5          
            if(PCount5<0) Dir5=NEG;                 // set direction negative
            else Dir5=POS;                          // set direction positive

            PCount5=abs(PCount5);                   // Make Path position positive        
            if(PCount5!=0) period5=STimer/PCount5;   // calculate time spacing between positions       
            Stime5=micros();                        // start time 
           
    } // end if DataInPathBuf
} // end 




//**********************************************************
// Quadrature output #1
//**********************************************************
void Quad1() { 
uint8_t cstate;
      
        cstate=0;                               // get current state of quad output
        if(digitalRead(Quad1B_Pin)) cstate=0x01;
        if(digitalRead(Quad1A_Pin)) cstate |= 0x02;

        switch(cstate){
        case 0x00:                                        // state 00
            if(Dir1==POS) digitalWrite(Quad1A_Pin,HIGH);  // new state 10
            else digitalWrite(Quad1B_Pin,HIGH);           // new state 01
            break;
        case 0x01:                                        // state 01
            if(Dir1==POS) digitalWrite(Quad1B_Pin,LOW);   // new state 00
            else digitalWrite(Quad1A_Pin,HIGH);           // new state 11
            break;          
        case 0x02:                                        // state 10
            if(Dir1==POS) digitalWrite(Quad1B_Pin,HIGH);  // new state 11
            else digitalWrite(Quad1A_Pin,LOW);            // new state 00
            break;                  
        case 0x03:                                        // state 11
            if(Dir1==POS) digitalWrite(Quad1A_Pin,LOW);   // new state 01
            else digitalWrite(Quad1B_Pin,LOW);            // new state 10
            break;                                
        } //end switch                                                                       
     
        if(Dir1==POS) ++scount1;                  // update internal counter        
        else --scount1;                                                                     
           
        --PCount1;                              // decrement path position counter
            
}  

//**********************************************************
// Quadrature output #2
//**********************************************************
void Quad2() {
uint8_t cstate;
      
        cstate=0;                               // get current state of quad output
        if(digitalRead(Quad2B_Pin)) cstate=0x01;
        if(digitalRead(Quad2A_Pin)) cstate |= 0x02;

        switch(cstate){
        case 0x00:                                        // state 00
            if(Dir2==POS) digitalWrite(Quad2A_Pin,HIGH);  // new state 10
            else digitalWrite(Quad2B_Pin,HIGH);           // new state 01
            break;
        case 0x01:                                        // state 01
            if(Dir2==POS) digitalWrite(Quad2B_Pin,LOW);   // new state 00
            else digitalWrite(Quad2A_Pin,HIGH);           // new state 11
            break;          
        case 0x02:                                        // state 10
            if(Dir2==POS) digitalWrite(Quad2B_Pin,HIGH);  // new state 11
            else digitalWrite(Quad2A_Pin,LOW);            // new state 00
            break;                  
        case 0x03:                                        // state 11
            if(Dir2==POS) digitalWrite(Quad2A_Pin,LOW);   // new state 01
            else digitalWrite(Quad2B_Pin,LOW);            // new state 10
            break;                                
        } //end switch                                                                       
     
        if(Dir2==POS) ++scount2;                  // update internal counter        
        else --scount2;                                                                     
           
        --PCount2;                              // decrement path position counter
 
} 


//**********************************************************
// Quadrature output #3
//**********************************************************
void Quad3() {
uint8_t cstate;
                           
        cstate=0;                               // get current state of quad output
        if(digitalRead(Quad3B_Pin)) cstate=0x01;
        if(digitalRead(Quad3A_Pin)) cstate |= 0x02;

        switch(cstate){
        case 0x00:                                        // state 00
            if(Dir3==POS) digitalWrite(Quad3A_Pin,HIGH);  // new state 10
            else digitalWrite(Quad3B_Pin,HIGH);           // new state 01
            break;
        case 0x01:                                        // state 01
            if(Dir3==POS) digitalWrite(Quad3B_Pin,LOW);   // new state 00
            else digitalWrite(Quad3A_Pin,HIGH);           // new state 11
            break;          
        case 0x02:                                        // state 10
            if(Dir3==POS) digitalWrite(Quad3B_Pin,HIGH);  // new state 11
            else digitalWrite(Quad3A_Pin,LOW);            // new state 00
            break;                  
        case 0x03:                                        // state 11
            if(Dir3==POS) digitalWrite(Quad3A_Pin,LOW);   // new state 01
            else digitalWrite(Quad3B_Pin,LOW);            // new state 10
            break;                                
        } //end switch                                                                       
     
        if(Dir3==POS) ++scount3;                  // update internal counter        
        else --scount3;                                                                     
           
        --PCount3;                              // decrement path position counter
 
} 


//**********************************************************
// Quardrature output #4
//**********************************************************
void Quad4() {
uint8_t cstate;

    if(image_count_dis==0) {                    // send pulse if image system counts enabled
      
        cstate=0;                               // get current state of quad output
        if(digitalRead(Quad4B_Pin)) cstate=0x01;
        if(digitalRead(Quad4A_Pin)) cstate |= 0x02;

        switch(cstate){
        case 0x00:                                        // state 00
            if(Dir4==POS) digitalWrite(Quad4A_Pin,HIGH);  // new state 10
            else digitalWrite(Quad4B_Pin,HIGH);           // new state 01
            break;
        case 0x01:                                        // state 01
            if(Dir4==POS) digitalWrite(Quad4B_Pin,LOW);   // new state 00
            else digitalWrite(Quad4A_Pin,HIGH);           // new state 11
            break;          
        case 0x02:                                        // state 10
            if(Dir4==POS) digitalWrite(Quad4B_Pin,HIGH);  // new state 11
            else digitalWrite(Quad4A_Pin,LOW);            // new state 00
            break;                  
        case 0x03:                                        // state 11
            if(Dir4==POS) digitalWrite(Quad4A_Pin,LOW);   // new state 01
            else digitalWrite(Quad4B_Pin,LOW);            // new state 10
            break;                                
        } //end switch                                                                       
     
        if(Dir4==POS) ++scount4;                  // update internal counter        
        else --scount4;                                                                     
           
        --PCount4;                              // decrement path position counter
      } // end image on/off         

}


//**********************************************************
// Quardrature output #5
//**********************************************************
void Quad5() {                                    
uint8_t cstate;

    if(image_count_dis==0) {                    // send pulse if image system counts enabled
      
        cstate=0;                               // get current state of quad output
        if(digitalRead(Quad5B_Pin)) cstate=0x01;
        if(digitalRead(Quad5A_Pin)) cstate |= 0x02;

        switch(cstate){
        case 0x00:                                        // state 00
            if(Dir5==POS) digitalWrite(Quad5A_Pin,HIGH);  // new state 10
            else digitalWrite(Quad5B_Pin,HIGH);           // new state 01
            break;
        case 0x01:                                        // state 01
            if(Dir5==POS) digitalWrite(Quad5B_Pin,LOW);   // new state 00
            else digitalWrite(Quad5A_Pin,HIGH);           // new state 11
            break;          
        case 0x02:                                        // state 10
            if(Dir5==POS) digitalWrite(Quad5B_Pin,HIGH);  // new state 11
            else digitalWrite(Quad5A_Pin,LOW);            // new state 00
            break;                  
        case 0x03:                                        // state 11
            if(Dir5==POS) digitalWrite(Quad5A_Pin,LOW);   // new state 01
            else digitalWrite(Quad5B_Pin,LOW);            // new state 10
            break;                                
        } //end switch                                                                       
     
        if(Dir5==POS) ++scount5;                  // update internal counter        
        else --scount5;                                                                     
           
        --PCount5;                              // decrement path position counter
      } // end image on/off  
}

//**********************************************************
// Quadrature output routine for Motor channels 1,2,3
//**********************************************************
void MotorQuadOutput() {
uint32_t Itime;
uint32_t Ctime;

      Ctime=micros();           // get current time
      
// motor 1 
  if(PCount1){                  // check for counts remaining
       
      if(Ctime>Stime1) {          // check for rollover
        Itime=Ctime-Stime1;       // time passed 
      
        if(Itime>period1){
          Stime1=Stime1+period1;
          Quad1();
        }
      }
  }
  
// motor 2 
  if(PCount2){                  // check for counts remaining
       
      if(Ctime>Stime2) {          // check for rollover
        Itime=Ctime-Stime2;       // time passed 
      
        if(Itime>period2){
          Stime2=Stime2+period2;
          Quad2();
        }
      }
  }
  
// motor 3
  if(PCount3){                  // check for counts remaining
       
      if(Ctime>Stime3) {          // check for rollover
        Itime=Ctime-Stime3;       // time passed 
      
        if(Itime>period3){
          Stime3=Stime3+period3;
          Quad3();
        }
      }
//      Serial.println(micros()-Ctime); //TEST*************
  }


  
}

//**********************************************************
// Quadrature output routine for Image System channels 4,5
//**********************************************************
void ImageQuadOutput() {
uint32_t Itime;
uint32_t Ctime;

      Ctime=micros();           // get current time
      
// image 1 (chan 4) 
  if(PCount4){                  // check for counts remaining
       
      if(Ctime>Stime4) {          // check for rollover
        Itime=Ctime-Stime4;       // time passed 
      
        if(Itime>period4){
          Stime4=Stime4+period4;
          Quad4();
        }
      }
  }
  
// image 2 (chan 5)
  if(PCount5){                  // check for counts remaining
       
      if(Ctime>Stime5) {          // check for rollover
        Itime=Ctime-Stime5;       // time passed 
      
        if(Itime>period5){
          Stime5=Stime5+period5;
          Quad5();
        }
      }
  }
  
}


//**********************************************************
// Send counts to Servo system channel 1
// Assumes motiion is stopped
//**********************************************************
void SendCount1(int16_t counts) {
int32_t Itime;
uint32_t Stime,Ctime;

        PCount1=counts;
        if(PCount1<0) Dir1=NEG;                 // set direction negative
        else Dir1=POS;                          // set direction positive

        PCount1=abs(PCount1);                   // Make Path position positive
        
        if(PCount1!=0) period1=STimer/PCount1;   // calculate time spacing between positions              

        Stime=micros();

        while(PCount1) {
          Ctime=micros();          // get current time
          
          if(Ctime>Stime) {        // check for rollover
          Itime=Ctime-Stime;       // time passed 
                     
            if(Itime>period1){
              Stime=Stime+period1;
              Quad1();
            }
          }
        }  // end while                              
} 

//**********************************************************
// Send counts to Servo system channel 2
// Assumes motiion is stopped
//**********************************************************
void SendCount2(int16_t counts) {
int32_t Itime;   
uint32_t Stime,Ctime;

        PCount2=counts;
            
        if(PCount2<0) Dir2=NEG;                 // set direction negative
        else Dir2=POS;                          // set direction positive

        PCount2=abs(PCount2);                   // Make Path position positive
        
        if(PCount2!=0) period2=STimer/PCount2;   // calculate time spacing between positions      
        
        Stime=micros();

        while(PCount2) {
          Ctime=micros();        // get current time
          
          if(Ctime>Stime) {        // check for rollover
          Itime=Ctime-Stime;       // time passed 
                     
            if(Itime>period2){
              Stime=Stime+period2;
              Quad2();
            }
          }
        }  // end while                    

} 

//**********************************************************
// Send counts to Servo system channel 3
// Assumes motiion is stopped
//**********************************************************
void SendCount3(int16_t counts) {
int32_t Itime;   
uint32_t Stime,Ctime;

        PCount3=counts;
            
        if(PCount3<0) Dir3=NEG;                     // set direction negative
        else Dir3=POS;                              // set direction positive

        PCount3=abs(PCount3);                   // Make Path position positive
        
        if(PCount3!=0) period3=STimer/PCount3;   // calculate time spacing between positions      

        Stime=micros();

        while(PCount3) {
          Ctime=micros();        // get current time
          
          if(Ctime>Stime) {        // check for rollover
          Itime=Ctime-Stime;       // time passed 
                     
            if(Itime>period3){
              Stime=Stime+period3;
              Quad3();
            }
          }
        }  // end while          

} 


//**********************************************************
// Send counts to image system channel 4
// Assumes motiion is stopped
//**********************************************************
void SendCount4(int16_t counts) {
int32_t Itime;   
uint32_t Stime,Ctime;

        PCount4=counts;
            
        if(PCount4<0) Dir4=NEG;                 // set direction negative
        else  Dir4=POS;                         // set direction positive

        PCount4=abs(PCount4);                   // Make Path position positive
        
        if(PCount4!=0) period4=STimer/PCount4;   // calculate time spacing between positions      

        Stime=micros();

        while(PCount4) {
          Ctime=micros();        // get current time
          
          if(Ctime>Stime) {        // check for rollover
          Itime=Ctime-Stime;       // time passed 
                     
            if(Itime>period4){
              Stime=Stime+period4;
              Quad4();
            }
          }
        }  // end while  
} 


//**********************************************************
// Send counts to image system channel 5
// Assumes motiion is stopped
//**********************************************************
void SendCount5(int16_t counts) {
int32_t Itime;   
uint32_t Stime,Ctime;

        PCount5=counts;
            
        if(PCount5<0) Dir5=NEG;                 // set direction negative
        else Dir5=POS;                          // set direction positive

        PCount5=abs(PCount5);                   // Make Path position positive       

        if(PCount5!=0) period5=STimer/PCount5;   // calculate time spacing between positions       
        
        Stime=micros();

        while(PCount5) {
          Ctime=micros();        // get current time
          
          if(Ctime>Stime) {        // check for rollover
          Itime=Ctime-Stime;       // time passed 
                     
            if(Itime>period5){
              Stime=Stime+period5;
              Quad5();
            }
          }
        }  // end while                     

} 


//**********************************************************
// Start motor motion
void StartMotion() {
 
  if(remote_mode==ON)
        status_byte|=0x08;              // error - cannot start motion in remote mode
    else { 
        jog_mode=0;                     // turn off jog mode
        motion=ON;
        STimer=SPATH_TIMER;
        PathTimer=PATH_TIMER;                                                                                                              
        Timer1.initialize(PathTimer);        // start 60Hz timer
        Timer1.attachInterrupt(NewPosition);   // attach new position interrupt
    } 
}

//**********************************************************
// Stop motor motion abruptly
void StopMotion() {  
    Timer1.detachInterrupt();                  // turn off 60Hz interrupt
    jog_mode=0;
    motion=OFF;
}

//**********************************************************
// Stop motor motion smoothly
void StopMotionSmoothly() {
    jog_mode=0;
    Timer1.detachInterrupt();                  // turn off 60Hz interrupt
}

//**********************************************************
// Start Jog Mode
void StartJogMode() {                                   // Start Jog Mode

    InitPathBuf();                                      // clear path buffer  
    jog_mode=ON;                                         // turn on jog mode
    motion=ON;       
    STimer=SPATH_TIMER;
    PathTimer=PATH_TIMER; 
    Timer1.initialize(PATH_TIMER);                    // start 60Hz timer
    Timer1.attachInterrupt(NewPosition);             // attach new position interrupt    
}

//**********************************************************
// Stop Jog Mode
void StopJogMode() {                                    // Stop Jog Mode
    Timer1.detachInterrupt();                        // turn off 60Hz interrupt
    InitPathBuf();                                      // clear path buffer
    jog_mode=OFF;                                       // turn off jog mode
    motion=OFF;    
}

//**********************************************************
// Encoder motion error
//**********************************************************
void EncoderMotionError() {
      
    if(enc_error_band1) {           // test for encoder 1 error, 0=OFF        
        if(abs(Enc1.read()-(scount1*step_rate)) > enc_error_band1) {
            if(enc_error_flg==0) enc_error_flg=1; // latch error
        }
    }        
    if(enc_error_band2) {           // test for encoder 2 error
        if(abs(Enc2.read()-(scount2*step_rate)) > enc_error_band2) {
            if(enc_error_flg==0) enc_error_flg=1; // latch error      
        } 
    }        
    if(enc_error_band3) {           // test for encoder 3 error
        if(abs(Enc3.read()-(scount3*step_rate)) > enc_error_band3) {
            if(enc_error_flg==0) enc_error_flg=1; // latch error  
        }
     }
              
      
}


