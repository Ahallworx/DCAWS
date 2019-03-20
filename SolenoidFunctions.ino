void setupSolenoids()
{
  // set solenoid pins to output
  pinMode(SOLENOID_1, OUTPUT);
  pinMode(SOLENOID_2, OUTPUT);
  pinMode(SOLENOID_3, OUTPUT);
  // write to High or open so they can be compressed
  digitalWrite(SOLENOID_1, HIGH);
  digitalWrite(SOLENOID_2, HIGH);
  digitalWrite(SOLENOID_3, HIGH);
  radio.println(F("Solenoids setup"));/////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void takeSample(int count)
{
 int solenoidPin;
 if(initSample)
 { 
    if(count == 1)
      solenoidPin = SOLENOID_1;
    if(count == 2)
      solenoidPin = SOLENOID_2;
    if(count == 3)
      solenoidPin = SOLENOID_3; 
    digitalWrite(solenoidPin, HIGH);
    initSample = false;
    sinceTrigger = 0;
 }
 else if(sinceTrigger >= HOLD_TIME)
 {
  digitalWrite(solenoidPin, LOW);
  targetCount++;
  initDepthHold = true; 
  initSample = true;
 }
}

