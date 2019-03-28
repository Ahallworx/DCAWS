void setupRadio()
{
  radio.begin(115200);
  checkRadio();
}

void checkRadio(void)
{
    radio.println(F("Press 'c' to confirm DCAWS to Radio comms"));
    while(!radio.available())
    {   
    }
    char key = (char)radio.read();
    if (key == 'c')
      radio.println(F("Comms confirmed"));
    else
    {
      radio.println(F("Either a key other than 'c' was pressed or unable to confirm comms!"));
      missionReady = false;
    }
    while(radio.available())
    {
      radio.read();
    }
}
