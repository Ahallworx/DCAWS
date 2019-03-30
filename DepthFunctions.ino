void getDepth()
{
  int sensorValue;                                                    
  double sensorVoltage;                                            
  double pressurePsiAbs;                                              
  double pressurePsiGage;                                           
  double pressurePa;                                                  
  sensorValue = analogRead(DEPTH_SENSOR);
  sensorVoltage = (((double)sensorValue/RES)*V_TEENSY)*1.5;
  pressurePsiAbs = ((sensorVoltage -.5)/4.0)*100.0;
  pressurePsiGage = pressurePsiAbs - 14.7;
  pressurePa = pressurePsiGage*6894.76; //convert psi to pascal
  depth = pressurePa/(RHO*g)+ .35;                                                //CHANGED 3/28 ADD .35 OFFSET
}
void getTargetDepths()
{
  radio.println(F("send the three desired sampling depths in meters in descending order followed by a 'k'"));
  while(radio.available())
  {
    radio.read();
  }
  while(!radio.available())
  {
  }
  td1 = constrain(radio.parseFloat(), MIN_TD, MAX_TD);
  td2 = constrain(radio.parseFloat(), MIN_TD, MAX_TD);
  td3 = constrain(radio.parseFloat(), MIN_TD, MAX_TD);
    radio.print(F("The three target depths are: "));
    radio.print(td1); radio.print(F(" "));
    radio.print(td2); radio.print(F(" "));
    radio.println(td3);
  if(radio.read() == 'k')
  {
    targetDepth1 = td1 + OFFSET;
    targetDepth2 = td2 + OFFSET;
    targetDepth3 = td3 + OFFSET;
  }
  while(radio.available())
  {
    radio.read();
  }
}
void setTargetDepth(int count)
{
  if(count == 1)
    targetDepth = targetDepth1;
  if(count == 2)
    targetDepth = targetDepth2;
  if(count == 3)
    targetDepth = targetDepth3;  
}


// 64-bit double-precision moving average
// inputs:  latest depth (double), init (boolean)
// output:  depth moving average (double)
void mvavgDepth(double z,boolean mvInit)
{
  // These static variables are kept in memory at all time
  // and can only be used by this function
  static double zSum;    // cumulated depth
  static int n;   // actual window size
  static int k;   // circular buffer index

  // if this is the first call, cumulated z value = input z value
  // and window size = 1; buffer the values
  if (mvInit == true)
  {
    zSum = z;
    n = 1;
    k = 0;
    zBuffer[k] = z; 
  }
  // if this is not the first call
  else
  {
    // if we have collected less points than the window size
    // cumulate the value and increment the window size
    if (n < WINSZ_Z)
    {
      zSum += z;
      zBuffer[++k] = z;
      n++;
    }
    // if we have already collected more points than the window
    // size, add the latest record and remove the oldest one
    // we also replace the oldest record in the circular buffer
    // with the new one
    else
    {
      k++;
      k %= WINSZ_Z;
      zSum += (z - zBuffer[k]);
      zBuffer[k] = z;
    }
  }
  
  // Calculate and return the moving average
  avgDepth = zSum / (double)n;
}
