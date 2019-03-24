void setupRadio()
{
  radio.begin(115200);
}

void checkRadio(void)
{
    radio.println(F("Press 'C' to confirm DCAWS to Radio comms"));
    while(!radio.available())
    {
      
    }
    char key = (char)radio.read();
    if (key == 'C')
      radio.println(F("Comms confirmed"));
    else
    {
      radio.println(F("Either a key other than 'C' was pressed or unable to confirm comms!"));
      missionReady = false;
    }
    while(radio.available())
    {
      radio.read();
    }
}
