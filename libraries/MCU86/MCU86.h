#include "Arduino.h" 

// Global constants
#define SW_VERSION 0x10        // software version number
#define HW_ID 0x09             // hardware ID

#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
#define POS 0
#define NEG 1

#define BUFFER_SIZE 7200        // path buffer size (60 positons per second (720 = 120 sec))
#define CMD_SIZE 100            // maximum size of command buffer
#define TBUF_SIZE 100           // maximum size of serial transmit buffer
#define BOM 0xAA                // beginning of message

#define ENC_OFFSET 0x80000000    // encoder offset

#define PATH_TIMER 16666        // 60Hz = 16666 usec
#define SPATH_TIMER 15666        // 60Hz - 1000us dead time


// define I/O pins  (x86Duino pins)
#define Quad1A_Pin 8              // quadrature output pins
#define Quad1B_Pin 9
#define Quad2A_Pin 10 
#define Quad2B_Pin 11
#define Quad3A_Pin 12 
#define Quad3B_Pin 13

#define Quad4A_Pin 21            // image system quadrature pins 
#define Quad4B_Pin 22
#define Quad5A_Pin 23 
#define Quad5B_Pin 24

#define Motor_En_Pin 39          // Motor 1,2,3 enable
#define Image_En_Pin 38          // Iage system enable
#define Motor_Flt_Pin 4          // Motor 1,2,3 fault imput
#define EStop_Pin 7              // Emergency stop input

#define Addr_In_Pin 2            // address input
#define Addr_Out_Pin 3           // address output

#define Remote_X 25              // remote control
#define Remote_Y 26
#define Remote_Z 27
#define Remote_En 28             // remote enable low
#define A_Speed 1                // analog speed


// ****************************************************
// Prototypes located in Command.cpp
// ****************************************************
void BuildCmd();                 // load receive buffer
void DecodeCmd();                // decode comand
uint8_t Checksum(uint8_t *s,uint8_t length); // calculate check sum
void PutStatus(uint8_t status_request);      // put status in tx buffer
void InitSerCmd();
void ClearLatchedFlags();        // clear latched flags
void SoftReset();                // soft reset
void InitComVar();               // initialize command variables

// ****************************************************
// Prototypes located in Initialize.cpp
// ****************************************************
void InitEncoders();
void InitQuad();
void InitGPIO();
uint8_t GetServoFault();
void EnableServo(uint8_t iset);
uint8_t AddrIn();
void AddrOut(uint8_t i);
void InitRemote();

// ****************************************************
// Prototypes located in PathControl.cpp
// ****************************************************
void InitPathBuf();
void WritePathData();
int16_t PathSpaceAvail();
void StartMotion();
void StopMotion();
void StopMotionSmoothly();
void StartJogMode();
void StopJogMode();
void SendCount1(int16_t counts);
void SendCount2(int16_t counts);
void SendCount3(int16_t counts);
void SendCount4(int16_t counts);
void SendCount5(int16_t counts);
void MotorQuadOutput();
void ImageQuadOutput();
void EncoderMotionError();
// ****************************************************
// Prototypes located in Remote.cpp
// ****************************************************
void StopRemote();        // ****ADD REMOTE.CPP

