void setupSolenoids()
{
  // set solenoid pins to output
  pinMode(SOLENOID_1, OUTPUT);
  pinMode(SOLENOID_2, OUTPUT);
  pinMode(SOLENOID_3, OUTPUT);
  // write to low or closed
  digitalWrite(SOLENOID_1, LOW);
  digitalWrite(SOLENOID_2, LOW);
  digitalWrite(SOLENOID_3, LOW);
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
 if(sinceTrigger >= HOLD_TIME)
 {
  digitalWrite(solenoidPin, LOW);
  targetCount++;
  sampleTaken = true;
 }
}

