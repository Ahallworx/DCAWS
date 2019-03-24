
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

void retrieveData()
{
    dcawsLog = SD.open("dcawsLog.csv");
  if (dcawsLog) {
    radio.println("dcawsLog.csv:");

    // read from the file until there's nothing else in it:
    while (dcawsLog.available()) {
      radio.write(dcawsLog.read());
    }
    // close the file:
    //dcawsLog.close();
  } else {
    // if the file didn't open, print an error:
    radio.println("error opening dcawsLog.csv");
  }
}

void checkSD()
{
  char fil = 'x';
  if (!dcawsLog)
  {
    radio.println(F("SD card failed!"));
    //missionReady = false;
    return;
  }
  radio.print(F("SD card initialization confirmed and found file: "));
  if (dcawsLog)
    radio.println(F("dcawsLog"));
  radio.println("send the 'f' character to retrieve latest file data or 's' to skip");
  while(fil != 's')
  { 
    while(!radio.available())
    {
    } 
    fil = (char)radio.read();
    if(fil == 'f')
    {
      retrieveData();   
    }
  }
  while(radio.available())
  {
    radio.read();
  }
}

void logData()
{
  if(dcawsLog)
  {
    if(initLog)
    {
      dcawsLog.print("New Mission: ");
      dcawsLog.print("Time: ");
      dcawsLog.print(GPS.hour, DEC); dcawsLog.print(':');
      dcawsLog.print(GPS.minute, DEC); dcawsLog.print(':');
      dcawsLog.print(GPS.seconds, DEC); dcawsLog.print('.');
      dcawsLog.print(GPS.milliseconds);
      dcawsLog.print(" Date: ");
      dcawsLog.print(GPS.day, DEC); dcawsLog.print('/');
      dcawsLog.print(GPS.month, DEC); dcawsLog.print("/20");
      dcawsLog.println(GPS.year, DEC);
      initLog = false;
    }
    //File dcawsLog = SD.open("dcawsLog.csv", FILE_WRITE);  
    dcawsLog.println(dataString);
    dcawsLog.flush();  
  }
}



