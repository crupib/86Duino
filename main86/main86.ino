#include <MCU86.h>

#include "Arduino.h" 
//#include "MCU86.h"
#include <Encoder.h>
#include <TimerOne.h>

// Test variable
uint32_t test1;

// external variables
extern uint32_t PCount1;
extern uint8_t motion;
extern uint8_t module_addr,MotorQuadEn; 
extern uint8_t cmd_ready,ImageEn,enc_error_flg;


// debug port to host PC @ 9600 baud
// RS484 serial port 19200 baud


void setup() {
  InitGPIO();                      // initialize GPIO
  InitSerCmd();                    // initialize serial ports and commands
  InitEncoders();                  // initialize encoders
  InitRemote();                    // initialize remote
  
  InitPathBuf();                    // initialize path buffer
  InitQuad();                       // initialize quadrature output
  InitComVar();                     // initialize command variables
 
}

void loop() {
  if (Serial.available())           // check for debug serial input
    Testit();
                                    // check for serial input 
  if (Serial485.available()&&(!cmd_ready))
    BuildCmd();                     // build command
    
  if(cmd_ready)
    DecodeCmd();                     // decode command

 if(MotorQuadEn)    
    MotorQuadOutput();               // motor quadrature output
 if(ImageEn)
    ImageQuadOutput();               // image sys quadrature output

//  CheckRemote();                      // Check remote control

  if(module_addr!=0) {               // check for errors
    if(GetServoFault()){              // servo fault and emergency stop status
//      StopMotion();    
    }
    
    if(enc_error_flg){                // check encoder position error
//      StopMotion();    
    }      
  } // end if module_addr

}



// Test code
void Testit() {
byte inchar;
     inchar = Serial.read();
     Serial.println(inchar);	 
//     Serial.println(micros()-test1);
//      Serial.println(micros()-Ctime); //TEST
}
