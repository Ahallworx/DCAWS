void systemDiagnosis()
{
  radio.println(F("Running System Diagnostic"));
  checkSD();
  checkGPS();
  checkThruster();
  getDepth();
  radio.print(F("Depth sensor detecting depth of "));
  radio.println(depth);
  getInternalCurrent();
  radio.print(F("Internal current sensor detecting current of "));
  radio.println(current);
  if(!getInternalCurrent())
  {
    radio.println(F("Internal current is below acceptable value!")); 
    missionReady = false;
  }
  getInternalTemp();
  radio.print(F("Internal temperature sensor detecting temperature of "));
  radio.println(temperature);
  if(!getInternalTemp())
  { 
    radio.println(F("Internal temperature is above acceptable value!"));
    missionReady = false;
  }
  getInternalPressure();
  radio.print(F("Internal pressure sensor detecting pressure of "));
  radio.println(pressure);
  if(!getInternalPressure())
  {
    radio.println(F("Internal pressure is above acceptable value!"));
    missionReady = false;
  }
  getInternalLeak();
  radio.print(F("Internal leak sensor detecting voltage of "));
  radio.println(leakVoltage);
  if(getInternalLeak())
  {
    radio.println(F("No leak detected."));
  }
  else
  {
    radio.println(F("Leak detected!"));
    missionReady = false;
  }
  tapSenseMonitor();
  radio.print(F("Internal tap sense detecting voltages of "));
  radio.print(tSense3Voltage); radio.print(F(" "));
  radio.print(tSense2Voltage); radio.print(F(" "));
  radio.println(tSenseTopVoltage);
  if(tapSenseMonitor())
  {
    radio.println(F("No low cell detected."));
  }
  else
  {
    radio.println(F("Low cell detected!"));
    missionReady = false;
  }
  if(missionReady)
  {
    radio.println(F("No issues detected. Deployment recomended.")); 
  }
  else
  {
    radio.println(F("Issue(s) detected! Deployment not recomended."));
    radio.println(F("Please review error messages and resolve issue before proceeding."));
    
  }
}

