void setupSolenoids()
{
  char sole = 'x';
  // set solenoid pins to output
  pinMode(SOLENOID_1, OUTPUT);
  pinMode(SOLENOID_2, OUTPUT);
  pinMode(SOLENOID_3, OUTPUT);
  // write to High or open so they can be compressed
  radio.println("send the 'o' character to open solenoids and then a 'c' when ready to close");
  while(sole != 'c')
  { 
    while(!radio.available())
    {
    } 
    sole = (char)radio.read();
    if(sole == 'o')
    {
      digitalWrite(SOLENOID_1, HIGH);
      digitalWrite(SOLENOID_2, HIGH);
      digitalWrite(SOLENOID_3, HIGH);   
    }
  }
  // close solenoids
  digitalWrite(SOLENOID_1, LOW);
  digitalWrite(SOLENOID_2, LOW);
  digitalWrite(SOLENOID_3, LOW);
  while(radio.available())
  {
    radio.read();
  }
  //radio.println(F("Solenoids setup"));
}

void takeSample(int count)
{
 static int solenoidPin;
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

