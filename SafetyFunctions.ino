int getInternalCurrent()
{
  int sensorValue;
  double sensorVoltage;
  sensorValue = analogRead(CURR_SENSOR);
  sensorVoltage = ((double)sensorValue/RES)*V_TEENSY; 
  current = ((sensorVoltage - 1.65)/.11); 
  /*if (current < MIN_SAFE_CURR)// do we have min or max safe value
   *  return 0;
   *else
   *  return 1;
   */
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
  double sensorVoltageOut;
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

void TapSenseMonitor()
{
  digitalWrite(BATTERY_SENSOR_EN, HIGH);  //tapsense on
  int tSense3 = analogRead(THIRD_CELL_READ);  //adc read cell 3
  int tSense2 = analogRead(SECOND_CELL_READ); //adc read cell 2
  int tSenseTop = analogRead(TOP_CELL_READ);  //adc read cell top
  double tSense3_V = ((((double)tSense3*3)/RES)*5.99);      //Voltage cell 3. 
  double tSense2_V = ((((double)tSense2*3)/RES)*5.99);      //Voltage of cell 2.
  double tSenseTop_V = ((((double)tSenseTop*3)/RES)*5.99);  //Voltage of Top cell.
  digitalWrite(BATTERY_SENSOR_EN, LOW);   //tapsense off
}

void checkSafetySensors()
{
  errorString = "";
  if(!getInternalCurrent())
  {
    state = ABORT;
    radio.println("Current too low");
    //log cause of error here
    errorString += "Current too low";
  }
  if(!getInternalTemp())
  {
    radio.println("Internal Temp too high");
    state = ABORT;
    errorString += "Internal Temp too high";
  } 
  if(!getInternalPressure())
  {
    radio.println("Internal Pressure too low");
    state = ABORT;
    errorString += "Internal Pressure too low";
  }
  if(!getInternalLeak())///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    radio.println("leak");
    state = ABORT;
    errorString += "Leak detected";
  }
  //include power here errorString += "Power too low";
}

