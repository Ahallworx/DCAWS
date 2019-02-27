void setupGPS()
{
  //initialize the GPS
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS
  GPS.begin(9600);
  GPS_Serial.begin(9600);
  // Request RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate (1 Hz)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  // Request updates on antenna status
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
}

void getGPS()
{
  char c = GPS.read();
  if (GPS.newNMEAreceived())
  {
    if(GPS.parse(GPS.lastNMEA()))
    {
      if(GPS.fix)
      {
        newGPS = true;
        goodGPSCount++;
      }
    }
  }
}

void checkGPS()
{
  timeout = 0;
  while(!newGPS)
  {
    getGPS();
    if(timeout > MAX_GPS_TIME)
    {
      radio.println(F("Unable to confirm GPS Fix!"));
      missionReady = 0;
      break;
    }
  }
  radio.println(F("GPS Fix confirmed."));
}

void sendGPS()
{
  radio.println(F("The current GPS coordinates are "));
  radio.print(GPS.latitude,6);
  radio.print(F(", "));
  radio.println(GPS.longitude,6);
  radio.println(F("The current averaged GPS coordinates are "));
  radio.print(avglat,6);
  radio.print(F(", "));
  radio.println(avglon,6);
}

void logGPS()
{
  File DCAWS_GPS = SD.open("DCAWS_GPS.csv", FILE_WRITE);
  DCAWS_GPS.print(avglat,6);
  DCAWS_GPS.print(", ");
  DCAWS_GPS.println(avglon,6);
  DCAWS_GPS.close(); 
}

void mvavg_GPS(double lat, double lon, boolean mvinit, double *latmean, double *lonmean)
{
  // These static variables are kept in memory at all time
  // and can only be used by this function
  static double latsum, lonsum;    // cumulated latitudes/longitudes
  static int n;   // actual window size
  static int k;   // circular buffer index

  // if this is the first call, cumulated lat value = input lat/lon value
  // and window size = 1; buffer the values
  if (mvinit == true)
  {
    latsum = lat;
    lonsum = lon;
    n = 1;
    k = 0;
    latbuffer[k] = lat;
    lonbuffer[k] = lon;
  }
  // if this is not the first call
  else
  {
    // if we have collected less points than the window size
    // cumulate the value and increment the window size
    if (n < winsz_GPS)
    {
      latsum += lat;
      latbuffer[++k] = lat;
      lonsum += lon;
      lonbuffer[k] = lon;
      n++;
    }
    // if we have already collected more points than the window
    // size, add the latest record and remove the oldest one
    // we also replace the oldest record in the circular buffer
    // with the new one
    else
    {
      k++;
      k %= winsz_GPS;
      latsum += (lat - latbuffer[k]);
      latbuffer[k] = lat;
      lonsum += (lon - lonbuffer[k]);
      lonbuffer[k] = lon;
    }
  }
}
