void systemDiagnosis()
{
  if(radio.available())
    radio.println(F("Running System Diagnostic"));
  checkRadio();
  checkGPS();
  checkSD();
  checkThruster();
  getDepth();
  radio.print(F("Depth sensor detecting depth of "));
  radio.println(depth);
  getInternalCurrent();
  radio.print(F("Internal current sensor detecting current of "));
  //radio.println();
  if(!getInternalCurrent())
  {
    radio.print(F("Internal current is below acceptable value!")); 
    missionReady = 0;
  }
  getInternalTemp();
  radio.print(F("Internal temperature sensor detecting temperature of "));
  //radio.println();
  if(!getInternalTemp())
  { 
    radio.print(F("Internal temperature is above acceptable value!"));
    missionReady = 0;
  }
  getInternalPressure();
  radio.print(F("Internal pressure sensor detecting pressure of "));
  //radio.println();
  if(!getInternalPressure())
  {
    radio.print(F("Internal pressure is above acceptable value!"));
    missionReady = 0;
  }
  if(getInternalLeak())
  {
    radio.print(F("No leak detected."));
  }
  else
  {
    radio.print(F("Leak detected!"));
    missionReady = 0;
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

