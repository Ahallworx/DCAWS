void setupThruster()
{
  //set up esc and send stop signal
  servo.attach(ESC_RC_PW); //attach ESC pin to servo
  servo.writeMicroseconds(1500); // send 'stop' signal
  delay(1000); // delay to allow theESC to recognize stop signal 
  setupSD();
}

void checkThruster()
{
  if(!servo.attached())
  {
    radio.println(F("Unable to confirm ESC connection!"));
    missionReady = 0;
    return;
  }
  radio.println(F("ESC connection confirmed."));  
}

void ascend()
{
  servo.writeMicroseconds(1500); //if we just rely on pos buoy to surface
  /*//or if we want to ascend with thruster help
  servo.writeMicroseconds(1250); could be any number between 1100 and 1500
  can do the math to optimize*/
}

void sendPIDSignal()
{
  // convert thrust to PWM signal
  signalESC = (17.98561151*thrust)+1500;
  // account for outliers
  if(signalESC < 1500)
    signalESC = 1500;
  if(signalESC > 1900)
    signalESC = 1900;
  servo.writeMicroseconds(signalESC);
  // may need 'delay' to recieve signal
}

