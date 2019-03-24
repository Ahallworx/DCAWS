void setupSD()
{
  //Initialize the SD Card Slot
  radio.println("Initializing SD card...");
  pinMode(chipSelect, OUTPUT); //For the SD Card to work
  if (SD.begin(chipSelect))
  {
    radio.println("initialization successful!");
    dcawsLog = SD.open("dcawsLog.csv", FILE_WRITE);
  }
  else
  {
    radio.println("initialization failed!");
    radio.println(F("SD card setup"));
    //return;
  }
}

void checkSD()
{
  if (!dcawsLog)
  {
    radio.println(F("SD card failed!"));
    //missionReady = false;
    return;
  }
  radio.print(F("SD card initialization confirmed and found file: "));
  if (dcawsLog)
    radio.println(F("dcawsLog"));
}

void logData()
{
  if(dcawsLog)
  {
    //File dcawsLog = SD.open("dcawsLog.csv", FILE_WRITE);  
    dcawsLog.println(dataString);
    dcawsLog.flush();  
  }
}
