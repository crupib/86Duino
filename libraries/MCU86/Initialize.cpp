
#include "Arduino.h" 
#include "MCU86.h"
#include <Encoder.h>
#include <TimerOne.h>

// variables
uint8_t enc_error_flg=0;            // encoder error flag
uint8_t enc_overflow=0;             // encoder overflow flag
uint8_t fault_flg=0;                // servo & emergency stop fault flag
uint8_t ImageEn=1;                  // image enable flag
uint8_t MotorQuadEn=1;              // motor 1,2,3 count enable 

void encoder_isr(int flag);          // encoder over/under flow routine


//****************************************************************************
// initialize serial communications 
//****************************************************************************
void InitSerCmd(void) {
  Serial.begin(9600);                // initialize debug port
  Serial485.begin(19200);            // initialize RS485 com port  
}

//****************************************************************************
// initialize encoders
//****************************************************************************
void InitEncoders(void) {
  Enc1.begin(MODE_AB_PHASE);       // initialize encoder 1, quadrature
  Enc1.setDigitalFilter(10);        // filter, 10nsec (100nsec steps)
  Enc1.write(ENC_OFFSET);          // initialize encider offset
  Enc1.setRange(0xFFFFFFFF,true);  //   set upper range for overflow
  Enc1.attachInterrupt(encoder_isr);  // encoder interrupt routine  

  Enc2.begin(MODE_AB_PHASE);       // initialize encoder 2, quadrature
  Enc2.setDigitalFilter(10);        // filter, 20nsec (100nsec steps)
  Enc2.write(ENC_OFFSET);          // initialize encider offset
  Enc2.setRange(0xFFFFFFFF,true);  //   set upper range for overflow
  Enc2.attachInterrupt(encoder_isr);  // encoder interrupt routine 
  
  Enc3.begin(MODE_AB_PHASE);       // initialize encoder 3, quadrature
  Enc3.setDigitalFilter(10);        // filter, 20nsec (100nsec steps)
  Enc3.write(ENC_OFFSET);          // initialize encider offset
  Enc3.setRange(0xFFFFFFFF,true);  //   set upper range for overflow
  Enc3.attachInterrupt(encoder_isr);  // encoder interrupt routine   
}

//****************************************************************************
// initialize quadrature outputs 
//****************************************************************************
void InitQuad(void) {
  
  pinMode(Quad1A_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad1A_Pin,LOW);      // initialize output
  pinMode(Quad1B_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad1B_Pin,LOW);      // initialize output
  
  pinMode(Quad2A_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad2A_Pin,LOW);      // initialize output
  pinMode(Quad2B_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad2B_Pin,LOW);      // initialize output  
 
  pinMode(Quad3A_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad3A_Pin,LOW);      // initialize output
  pinMode(Quad3B_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad3B_Pin,LOW);      // initialize output 

  pinMode(Quad4A_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad4A_Pin,LOW);      // initialize output
  pinMode(Quad4B_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad4B_Pin,LOW);      // initialize output 

  pinMode(Quad5A_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad5A_Pin,LOW);      // initialize output
  pinMode(Quad5B_Pin,OUTPUT);        // Quadrature output pin
  digitalWrite(Quad5B_Pin,LOW);      // initialize output  
}

//****************************************************************************
// initialize GPIO 
//****************************************************************************
void InitGPIO() {
  pinMode(Motor_En_Pin,OUTPUT);         // Motor 1,2,3 enable output pin
  digitalWrite(Motor_En_Pin,HIGH);      // initialize output enabled 
  
  pinMode(Image_En_Pin,OUTPUT);         // Image enable output pin
  digitalWrite(Image_En_Pin,HIGH);      // initialize output enabled
 
  pinMode(Motor_Flt_Pin,INPUT_PULLUP);  // Motor 1,2,3 fault input

  pinMode(EStop_Pin,INPUT_PULLUP);      // emergency stop 

  pinMode(Addr_Out_Pin,OUTPUT);         // address output
  digitalWrite(Addr_Out_Pin,HIGH);    

  pinMode(Addr_In_Pin,INPUT_PULLUP);    // address input  
  
}

//****************************************************************************
// initialize Remote 
//****************************************************************************
void InitRemote() {
  pinMode(Remote_X,INPUT_PULLUP);      // remote x
  pinMode(Remote_Y,INPUT_PULLUP);      // remote y
  pinMode(Remote_Z,INPUT_PULLUP);      // remote z
  pinMode(Remote_En,INPUT_PULLUP);      // remote enable
  
}

//****************************************************************************
// encoder overflow interrupt service routine 
//****************************************************************************
void encoder_isr(int flag) {
    enc_overflow=TRUE;
    StopMotion();

}

//**********************************************************
// get servo fault status
//**********************************************************
uint8_t GetServoFault(void) {
uint8_t status;

    status=0;
    if(digitalRead(Motor_Flt_Pin))          // Check motor fault
        status|=0x01;
    if(digitalRead(EStop_Pin))
        status|=0x08;                       // Check emergency stop
   
    if(status)                              // latch fault flag
        fault_flg=ON;
        
    return(status);
}

//**********************************************************
// Disable / Enable servo 1, 2, 3 and Image system
//**********************************************************
void EnableServo(uint8_t iset) {

    if(iset & 0x10)
        digitalWrite(Motor_En_Pin,HIGH);      // enable motors  
    else
        digitalWrite(Motor_En_Pin,LOW);       // disable motors
           

     if(iset & 0x80)  
        digitalWrite(Image_En_Pin,HIGH);      // enable image sys
    else
        digitalWrite(Image_En_Pin,LOW);       // disable image sys
              
}


//**********************************************************
// read address in 
//**********************************************************
uint8_t AddrIn(void) {
    return(digitalRead(Addr_In_Pin));
}

//**********************************************************
// set address out
//**********************************************************
void AddrOut(uint8_t i) {
  digitalWrite(Addr_Out_Pin,i); 
}


