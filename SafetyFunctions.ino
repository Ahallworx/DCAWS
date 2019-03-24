int getInternalCurrent()
{
  int sensorValue;
  double sensorVoltage;
  sensorValue = analogRead(CURR_SENSOR);
  sensorVoltage = ((double)sensorValue/RES)*V_TEENSY; 
  current = ((sensorVoltage - 1.65)/.11); 
  if (current <= MIN_SAFE_CURR || current > MAX_SAFE_CURRENT)
    return 0;
  else
    return 1;
   
}

int getInternalTemp()
{
  int sensorValue;
  double sensorVoltage;
  sensorValue = analogRead(TEMPERATURE_SENSOR);
  sensorVoltage=((double)sensorValue/RES)*V_TEENSY;           
  temperature = ((sensorVoltage-.5)/.01);  
  if (temperature > MAX_SAFE_TEMP)
    return 0;
  else
    return 1;
}

int getInternalPressure()
{
  int sensorValue;
  double sensorVoltage;
  //double sensorVoltageOut;
  sensorValue= analogRead(INT_PRESS_SENSOR);
  sensorVoltage = (((double)sensorValue/RES)*V_TEENSY)*1.5;
  //sensorVoltageOut = ((sensorVoltage/4.5)*(.9392*V_SUPPLY))+(.04*V_SUPPLY);
  pressure = ((((sensorVoltage/*Out*/)/V_SUPPLY)+.04)/(0.004));
  /*if (pressure > MAX_SAFE_PRESS)
   *  return 0;
   *else
   *  return 1;
   */
}

int getInternalLeak()
{
  int sensorValue;
  sensorValue= analogRead(LEAK_SENSOR);
  leakVoltage = (sensorValue*(3.3/RES)); 
  if (leakVoltage < MIN_SAFE_LEAK_V)
    return 0;
  else
    return 1;
}

int tapSenseMonitor()
{
  int tSense3 = analogRead(THIRD_CELL_READ);  //adc read cell 3
  int tSense2 = analogRead(SECOND_CELL_READ); //adc read cell 2
  int tSenseTop = analogRead(TOP_CELL_READ);  //adc read cell top
  tSense3Voltage = ((((double)tSense3*V_TEENSY)/RES)*5.99);      //Voltage cell 3. 
  tSense2Voltage = ((((double)tSense2*V_TEENSY)/RES)*5.99);      //Voltage of cell 2.
  tSenseTopVoltage = ((((double)tSenseTop*V_TEENSY)/RES)*5.99);  //Voltage of Top cell.
  if(tSense3Voltage < MIN_SAFE_TSENSE || tSense2Voltage < MIN_SAFE_TSENSE || tSenseTopVoltage < MIN_SAFE_TSENSE)
    return 0;
  else
    return 1;
}

void checkSafetySensors()
{
  errorString = "";
  if(!getInternalCurrent())
  {
    state = ABORT;
    //log cause of error here
    errorString += "Current too low";
  }
  if(!getInternalTemp())
  {
    state = ABORT;
    errorString += "Internal Temp too high";
  } 
  if(!getInternalPressure())
  {
    state = ABORT;
    errorString += "Internal Pressure too low";
  }
  if(!getInternalLeak())
  {
    state = ABORT;
    errorString += "Leak detected";
  }
  if(!tapSenseMonitor())
  {
    state = ABORT;
    errorString += "Power too low";
  }
  
}

void setupTapSense()
{
  pinMode(BATTERY_SENSOR_EN, OUTPUT);
  digitalWrite(BATTERY_SENSOR_EN, HIGH);  //tapsense on
  radio.println(F("tap sense set up"));
}

