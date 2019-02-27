int getInternalCurrent()
{
  int adcVal_curr;
  adcVal_curr = analogRead(CURR_SENSOR);
  double curr_count = (adcVal_curr*(3.3/1024)); //convert ADC current value to Vout value.
  double current = (0.11/(adcVal_curr - 1.65)); //Current (A).
  double currVal3 = (3.33*(adcVal_curr/1024));  //
  double currVal5 = (5*(currVal3/3.3));
  /*if (currVal5 < MIN_SAFE_CURR)
   *  return 0;
   *else
   *  return 1;
   */
}

int getInternalTemp()
{
  int adcVal_temp;
  adcVal_temp = analogRead(TEMPERATURE_SENSOR);
  double temp_count=(1.65*(adcVal_temp/1024)+.1);  //convert ADC temp value to Vout value.          
  double temp = ((temp_count-.5)/.01);   //Temp in C.
   /*if (temp > MAX_SAFE_TEMP)
   *  return 0;
   *else
   *  return 1;
   */
}

int getInternalPressure()
{
  int adcVal_inPress;
  adcVal_inPress= analogRead(INT_PRESS_SENSOR);
  //double pressure = (((currVal5/5.1)+.04)/(0.004)); //why is currVal used here?
  /*if (pressure > MAX_SAFE_PRESS)
   *  return 0;
   *else
   *  return 1;
   */
}

int getInternalLeak()
{
  int adcVal_leak;
  adcVal_leak= analogRead(LEAK_SENSOR);
  double leak = (adcVal_leak* (3/4096)); //why is this 4096 not 1024?
  /*if (leak > MIN_SAFE_LeakV (=3V))
   *  return 0;
   *else
   *  return 1;
   */
}

void checkSafetySensors()
{
  if(!getInternalCurrent())
  {
    state = ABORT;
    //log cause of error here
  }
  if(!getInternalTemp())
    state = ABORT;
  if(!getInternalPressure())
    state = ABORT;
  if(getInternalLeak())
    state = ABORT;
  //include power here
}

