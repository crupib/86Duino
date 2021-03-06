#include "Arduino.h" 
#include "MCU86.h"
#include <Encoder.h>
#include <TimerOne.h>


// external variables
extern uint8_t cmd_ready;                // command ready
extern int32_t scount1, scount2, scount3, scount4, scount5; // internal counters
extern int16_t PathIn[3], PathOut[3];     // path input and output buffers
extern int16_t DataInPathBuf;              // number of paths in buffer
extern uint32_t PathsExecuted;           // total number of paths executed
extern uint32_t PathsPlusBuf;            // total number of paths plus buffer

extern uint8_t last_path_flg;          // last path flag
extern uint8_t enc_error_flg;          // encoder error flag
extern uint8_t enc_overflow;           // encoder overflow flag
extern uint8_t fault_flg;              // servo and emergency stop flag
extern uint8_t path_error;             // path buffer error
extern uint8_t ImageEn;                // enable counts to image system
extern uint8_t MotorQuadEn;            // motor 1k,2,3 counts enable

extern uint8_t motion;                // motion mode
extern uint8_t jog_mode;              // jog mode
extern uint8_t remote_mode;          // remote mode

// variables
uint8_t status_request;               // status request flag
uint8_t group_leader;               // group leader flag
uint8_t cmd[CMD_SIZE];
uint8_t module_addr;                // UT Interface Module address, default=0
uint8_t group_addr = 0xFF;            // Group address, defaulextern uint8_t ImageEnt =0xFF
uint8_t cmdlen;
uint8_t cmd_ready;                  // command ready


uint8_t status_byte;                // status byte 
uint8_t status_packet;
uint8_t step_rate=1;                // step rate multiplier

uint8_t remote_dis;                  // remote disable flag

uint16_t  enc_error_band1;            // encoder motion error band
uint16_t  enc_error_band2;            // encoder motion error band
uint16_t  enc_error_band3;            // encoder motion error band

//****************************************************************************
// initialize command variables 
//****************************************************************************
void InitComVar(void) { 
  module_addr = 0;                   // initialize module address  
  group_addr = 0xFF;                 // initialize group address
  step_rate=1;                       // step rate
  enc_overflow=0;                    // encoder over flow flag
  ImageEn=1;                         // enable image system
}

//****************************************************************************
// build command 
//****************************************************************************
void BuildCmd(void) {

static uint8_t c_in=0;
uint8_t cb[1];
uint8_t c, pathcount;

     Serial485.readBytes(cb,1);          // get character
     c=cb[0];

     if(AddrIn() && (module_addr==0)) // check network address in
        return;                    // return if module is disabled

     switch(c_in) {

     case 0:                       // first byte must be BOM 
        if(c==BOM) 
          cmd[c_in++]=c;           // place BOM in buffer               
     break;


     case 1:                       // second byte must be module address
        if((c==module_addr)||(c==group_addr))
           cmd[c_in++]=c;          // place module address in buffer
        else 
           c_in=0;                 // clear index, not for this module 
     break;

     case 2:                       // third byte is data length and command 
        cmd[c_in++]=c;             // place data length and command in buffer 
        cmdlen=((c>>4)&0x0F)+4;    // calculate command length
        if(cmdlen>CMD_SIZE)        // check for max size
           c_in=0;
     break;

     default:
        cmd[c_in++]=c;             // place data in buffer
        if(c_in==cmdlen) {
           cmd_ready=1;            // set command ready flag  
           c_in=0;                 // reset command buffer index 
        }
     break;
     }
        
}

//****************************************************************************
// decode command 
//****************************************************************************
void DecodeCmd(void) {

uint8_t command,checklen,k;
uint8_t i,pathcount;
int32_t eset,tset;
int16_t iset, cset;
uint16_t xset;  

  status_byte=0;                      // clear status byte
  
  if((cmd[1]==module_addr)||group_leader)
     status_request=1;                // return status packet
  else
     status_request=0;                // do not return status packet

  checklen=(((cmd[2]>>4)&0x0F)+2);    // calculate checksum byte length

  k=Checksum(&cmd[1],checklen);       // validate checksum

  if(k!=cmd[checklen+1]) {
     status_byte|=0x02;               // set check sum error bit
     if(status_request)
        PutStatus(status_packet);     // request status packet
     cmd_ready=0;                     // clear command ready flag 
     return;                          // return checksum error   
  }
  
  command=cmd[2] & 0x0F;              // mask command byte

  switch(command){

//***************************************************************************
  case 0x00:                          // Set encoders  
    i=4;                   
    tset=0;
    eset=0; 
    tset=cmd[i++];
    eset=eset | tset;                   // load LSB
    tset=cmd[i++]<<8;                
    eset=eset | tset;                   // load 2d=nd byte
    tset=cmd[i++]<<16;
    eset=eset | tset;                   // load 3rd byte
    tset=cmd[i++]<<24;
    eset=eset | tset;                   // load MSB        

    if(cmd[3] & 0x01) Enc1.write(eset+ENC_OFFSET);  // set encoder 1 
    if(cmd[3] & 0x02) Enc2.write(eset+ENC_OFFSET);  // set encoder 2 
    if(cmd[3] & 0x04) Enc3.write(eset+ENC_OFFSET);  // set encoder 3 
    if(cmd[3] & 0x08) scount1=eset;      // set servo 1 internal counter
    if(cmd[3] & 0x10) scount2=eset;      // set servo 2 internal counter
    if(cmd[3] & 0x20) scount3=eset;      // set servo 3 internal counter
    if(cmd[3] & 0x40) scount4=eset;      // set servo 4 internal counter
    if(cmd[3] & 0x80) scount5=eset;      // set servo 5 internal counter
                      
    if(status_request)
        PutStatus(status_packet);         // send status packet
  break;

//***************************************************************************
  case 0x01:                          // set address
     
     module_addr=cmd[3];              // get module address

     if((cmd[4]&0x80)==0)             // check MSB for group leader
        group_leader=1;
     else
        group_leader=0;   
 
     group_addr=cmd[4]|0x80;          // get group address, set MSB

     if(module_addr!=0) 
        AddrOut(0);                  // enable next device on network     
     else
        AddrOut(1);                  // disable next device on network
     if(status_request)
        PutStatus(status_packet);     // send status packet
        

  break;
//***************************************************************************
  case 0x02:                          // define status
     status_packet=cmd[3];            // set status packet request register
     if(status_request)
        PutStatus(status_packet);     // send status packet
  break;
//***************************************************************************
  case 0x03:                          // read status
    if(status_request)
        PutStatus(cmd[3]);            // send requested status packet
  break;
/***************************************************************************/
  case 0x04:                           // Jog Mode
    if(cmd[3]&0x01)
        StartJogMode();                // Start Jog Mode
    if(cmd[3]&0x02)
        StopJogMode();                 // Stop Jog Mode

    if(status_request)
        PutStatus(status_packet);      // send status packet
  break;
//***************************************************************************
  case 0x05:                          // Sart Motion
    StartMotion();       
       
    if(status_request)
        PutStatus(status_packet);      // send status packet
  break;
//***************************************************************************
  case 0x06:                          // Step Rate Multiplier
    step_rate=cmd[3];                 // set step rate multiplier
 
    if(status_request)
        PutStatus(status_packet);      // send status packet   
    
  break;
//***************************************************************************
  case 0x07:                           // Stop Motion
  
    if(cmd[3]&0x01)
        ImageEn=0;            // disable counts to image system
    else
        ImageEn=1;            // enable counts to image system

      if(cmd[3]&0x02)
        InitPathBuf();                 // reset path position buffer

    if(cmd[3] & 0x04) {                // Stop motion abruptly
        StopMotion();      
    }

    if(cmd[3] & 0x08) {                 // Stop motion smoothly
        StopMotionSmoothly();
    }

    if(cmd[3]&0x40)
        MotorQuadEn=0;            // disable counts to image system
    else
        MotorQuadEn=1;            // enable counts to image system

    EnableServo(cmd[3]);                // enable/disable servos and image system
    
    if(status_request)
        PutStatus(status_packet);      // send status packet
  break;
//***************************************************************************
  case 0x08:                                // Test for encoder position error

    xset=(cmd[5]<<8) | cmd[4];              // combine LSB and MSB, load Path buffer

    if(cmd[3]&0x01) enc_error_band1=xset;  //set error count band
    if(cmd[3]&0x02) enc_error_band2=xset;
    if(cmd[3]&0x04) enc_error_band3=xset;           

    if(status_request)
        PutStatus(status_packet);           // send status packet
  break;
//***************************************************************************
  case 0x09:                                // Remote control


    if(cmd[3]&0x02) {                         // remote control
        remote_dis=1;                        // disable remote
        StopRemote();                       // stop remote mode
    }
    else
        remote_dis=0;                        // enable remote


    if(status_request)
        PutStatus(status_packet);           // send status packet

  break;
//***************************************************************************
  case 0x0A:                          // set baud rate

  switch(cmd[3]) {
    case 127:                                   // 9600 BAUD
      Serial485.end();
      Serial485.begin(9600);                                       
    break;

    case 64:                                    // 19200 baud
      Serial485.end();
      Serial485.begin(19200);            
    break;

    case 21:                                     // 57600 baud
      Serial485.end();
      Serial485.begin(57600);        
    break;
 
    case 10:                                     // 115,200 baud
      Serial485.end();
      Serial485.begin(115200);       
    break;

    case 5:                                      // 500,000 baud
      Serial485.end();
      Serial485.begin(500000);         
    break;

    case 1:                                      // 1,000,000 baud
      Serial485.end();
      Serial485.begin(1000000);                          
    break;     
 
    case 0:                                      // 3,000,000 baud
      Serial485.end();
      Serial485.begin(3000000);                  
              
    break;  
 
  }

    
    if(status_request)
        PutStatus(status_packet);      // send status packet
  break;
//***************************************************************************
  case 0x0B:                           // clear latched status bits
  
    ClearLatchedFlags();
   
    if(status_request)
        PutStatus(status_packet);      // send status packet

  break;
//***************************************************************************
  case 0x0C:                             // send counts to servos and image system

        i=4;
        
        cset=0;
        iset=0; 
        cset=cmd[i++];
        iset=iset | cset;               // load LSB
        cset=cmd[i++]<<8;                
        iset=iset | cset;               // load 2nd byte
        
        if(cmd[3] & 0x01)               // send counts to servo channel 1
            SendCount1(iset);
             
        if(cmd[3] & 0x02)               // send counts to servo channel 2
            SendCount2(iset);         
 
        if(cmd[3] & 0x04)               // send counts to servo channel 3
            SendCount3(iset);
                      
        if(cmd[3] & 0x08)               // send counts to image system channel 4
            SendCount4(iset);               

        if(cmd[3] & 0x10)               // send counts to image system channel 5
            SendCount5(iset);  

    if(status_request)
        PutStatus(status_packet);      // send status packet

  break;

//***************************************************************************
  case 0x0D:                             // Add path points to path buffer

    i=0; 
    pathcount=1;                            // 1 path per command
//    pathcount=(cmd[2]>>4)&0x0F;          // get number of paths to store
                                            // use for multiple paths per command 
    while(pathcount>0) {
        PathIn[0]=0;
        PathIn[0]=(cmd[4+i]<<8) | cmd[3+i];  // combine LSB and MSB, load Path buffer
        PathIn[1]=0;
        PathIn[1]=(cmd[6+i]<<8) | cmd[5+i];
        PathIn[2]=0;    
        PathIn[2]=(cmd[8+i]<<8) | cmd[7+i];
        PathIn[3]=0;
        PathIn[3]=(cmd[10+i]<<8) | cmd[9+i]; 
        PathIn[4]=0;
        PathIn[4]=(cmd[12+i]<<8) | cmd[11+i];
        i+=10;                               // update path counter

        WritePathData();                 // store path in buffer
        --pathcount;                     // decrement path count     
    }

     if(status_request)
        PutStatus(status_packet);        // request status packet        

        
  break;


//***************************************************************************
  case 0x0E:                             // NOP
     if(status_request)
        PutStatus(status_packet);        // request status packet
  break;
//***************************************************************************
  case 0x0F:                             // soft reset
     
    SoftReset(); 
  
  break;
//***************************************************************************


  default:                          


  break;

//**************************************************************************

  } //end switch
  
  cmd_ready=0;                             // clear command ready flag 

}



//****************************************************************************
// calculate 8 bit checksum
//****************************************************************************

uint8_t Checksum(uint8_t *s,uint8_t length) {
    
uint8_t checksum,i;

  checksum=0;
  for(i=0;i<length;i++)
     checksum=checksum + s[i];
  return(checksum);
}

//****************************************************************************
// put status packet into transmit buffer
//****************************************************************************
void PutStatus(uint8_t status_request) {

uint8_t stat[TBUF_SIZE];                // status buffer
uint8_t i;
uint32_t enc,clk;
uint16_t k;

//uint32_t enctst; // TEST *****


  i=0;

  if(path_error)                        // path buffer data error
        status_byte |= 0x01;
  if(fault_flg)
        status_byte |= 0x04;            // servo / emergency stop fault - latched
//  if(remote_sw==ON)
//        status_byte|=0x08;              // remote switch status      
        
  if(last_path_flg)
        status_byte |= 0x10;            // last path executed flag - latched
  if(enc_overflow)                        // encoder overflow
        status_byte |= 0x20;       
  if(enc_error_flg)                     // encoder position error - latched
        status_byte |= 0x40;
  if(GetServoFault())                   // check for servo error
        status_byte |= 0x80;       


  stat[i++]=status_byte;                // load status byte

  if(status_request & 0x01){            // fetch encoder 1 data
     enc = Enc1.read()-ENC_OFFSET;
        enc *= step_rate;
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB

     enc = Enc2.read()-ENC_OFFSET;      // fetch encoder 2 data
        enc *= step_rate;
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB
        
     enc = Enc3.read()-ENC_OFFSET;      // fetch encoder 3 data
        enc *= step_rate;
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB
    } //end if
     
  if(status_request & 0x02){            // fetch servo 1 counter data
     enc = scount1*step_rate;
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB

     enc = scount2*step_rate;            // fetch servo 2 counter  data
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB
        
     enc = scount3*step_rate;            // fetch servo 3 counter  data
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB
    } // end if

  if(status_request & 0x04){            // fetch servo 4 (image sys) counter data
     enc = scount4*step_rate;
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB

     enc = scount5*step_rate;            // fetch servo 5 (image sys) counter  data
        stat[i++]= lowByte(enc);        // store LSB
        stat[i++]= highByte(enc);       // 2nd byte
        enc=enc>>16;
        stat[i++]= lowByte(enc);        // 3rd byte
        stat[i++]= highByte(enc);       // MSB
  } // end if
  

//    if(status_request & 0x08){          // Unused
//    }
    

 
    if(status_request & 0x10){          // send total paths executed plus paths in buffer
     
        PathsPlusBuf=PathsExecuted+DataInPathBuf;  // total number of paths executed + paths in buffer

        stat[i++]= PathsPlusBuf & 0x000000FF;    // store LSB
        PathsPlusBuf=PathsPlusBuf>>8;
        stat[i++]= PathsPlusBuf & 0x000000FF;    // 2nd byte
        PathsPlusBuf=PathsPlusBuf>>8;
        stat[i++]= PathsPlusBuf & 0x000000FF;    // 3rd byte
        PathsPlusBuf=PathsPlusBuf>>8;
        stat[i++]= PathsPlusBuf & 0x000000FF;    // MSB    
     
    }

   if(status_request & 0x20){       // send device ID and SW version
        stat[i++]=HW_ID;            // load hardware ID
        stat[i++]=SW_VERSION;       // load software version number
    }
    
   if(status_request & 0x40){       // send I/O byte
        stat[i++]=GetServoFault();  // get servo fault status
    }
    
   if(status_request & 0x80){       // send path positions space available
        k=PathSpaceAvail();         // get space available
        stat[i++]=k & 0x00FF;       // get LSB
        k = k>>8;
        stat[i++]=k & 0x00FF;       // get MSB
    }

    k=Checksum(&stat[0],i);          // calculate checksum
      stat[i++]=k;                  // send checksum

     Serial485.write(stat,i);        // transmit status packet

}

//****************************************************************************
// clear latched flags
//****************************************************************************
void ClearLatchedFlags(void) {
    status_byte=0;     
    enc_error_flg=0;                   // clear encoder error flag
    enc_overflow=0;                    // clear encoder overflow flag
    fault_flg=0;                       // clear servo & emergency fault flag
    last_path_flg=0;                   // clear last path executed flag
    path_error=0;                      // clear path error   
}



//****************************************************************************
// soft reset
//****************************************************************************
void SoftReset(void) {
  
    StopMotion();
    InitGPIO();                      // initialize GPIO
  
    InitSerCmd();                    // initialize serial ports and commands
    InitEncoders();                  // initialize encoders
    InitRemote();                    // initialize remote
  
    InitPathBuf();                    // initialize path buffer
    InitQuad();                       // initialize quadrature output
    InitComVar();                     // initialize command variables
    
    module_addr=0; 
    status_packet=0; 
    group_addr = 0xFF;
    
    remote_dis=0;
    ImageEn=0;              // enable image system counts
    motion=0;                       // motion = off
    jog_mode=0;                     // jog mode = off
    remote_mode=0;                  // remote mode = off 
    
    scount1=0;               // initialize internal counters
    scount2=0; 
    scount3=0;
    scount4=0;
    scount5=0;
        
    ClearLatchedFlags();     
}



