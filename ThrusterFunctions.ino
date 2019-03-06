void setupThruster()
{
  //set up esc and send stop signal
  servo.attach(ESC_RC_PW); //attach ESC pin to servo
  servo.writeMicroseconds(STOP_SIGNAL); // send 'stop' signal
  delay(1000/*SETUP_DELAY*/); // delay to allow theESC to recognize stop signal 
}

void checkThruster()
{
  if(!servo.attached())
  {
    radio.println(F("Unable to confirm ESC connection!"));
    missionReady = false;
    return;
  }
  radio.println(F("ESC connection confirmed."));  
}

void ascend()
{
  servo.writeMicroseconds(STOP_SIGNAL); //if we just rely on pos buoy to surface
  /*//or if we want to ascend with thruster help
  servo.writeMicroseconds(1250); could be any number between 1100 and 1500
  can do the math to optimize*/
}

void sendPIDSignal()
{
  // convert thrust to PWM signal
  signalESC = (T_GAIN*thrust)+STOP_SIGNAL;
  // account for outliers
  if(signalESC < STOP_SIGNAL)
    signalESC = STOP_SIGNAL;
  if(signalESC > 1900)
    signalESC = 1900;
  servo.writeMicroseconds(signalESC);
  // may need 'delay' to recieve signal
}

