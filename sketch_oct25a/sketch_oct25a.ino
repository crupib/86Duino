#include <mysketch.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  print_int myint(100);
  Serial.println(myint.GetInt());
}
