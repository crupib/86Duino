#ifndef mysketch_h
#define mysketch_h
#include "Arduino.h" 
class print_int
{
   public:
     print_int(int I);
     int GetInt();  
  private:
   int _myint;
  
};  
#endif
