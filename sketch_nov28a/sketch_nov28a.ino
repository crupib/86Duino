int incomingByte; 
void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   incomingByte = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println( "vLUT Check Completed");
  incomingByte = Serial.read();
  if (incomingByte != -1)
  {
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
}
