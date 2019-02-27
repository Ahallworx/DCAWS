void setupSD()
{
  //Initialize the SD Card Slot
  radio.print("Initializing SD card...");
  pinMode(chipSelect, OUTPUT); //For the SD Card to work
  if (!SD.begin(chipSelect))
  {
    radio.println("initialization failed!");
    return;
  }
  DCAWS_Depth = SD.open("DCAWS_Depth.csv", FILE_WRITE);
  DCAWS_GPS = SD.open("DCAWS_GPS.csv", FILE_WRITE);
}

void checkSD()
{
  if (!SD.begin(chipSelect))
  {
    radio.println(F("SD card initialization failed!"));
    missionReady = 0;
    return;
  }
  radio.print(F("SD card initialization confirmed and found files: "));
  if (DCAWS_Depth)
    radio.print(F("DCAWS_Depth, "));
  if (DCAWS_GPS)
    radio.println(F("DCAWS_GPS"));
}
