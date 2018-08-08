#include "Arduino.h"
#include <mysketch.h>
/*
  mysketch.cpp - Library for testing
*/

print_int::print_int(int I)
{
        _myint =  I;
}

int print_int::GetInt() 
{
   return _myint;
}