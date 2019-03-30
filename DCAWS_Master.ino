// Alex Barker and Adam Hall
// 3/29/2019
// Code to run autonomous water sampler DCAWS
// Changes from previous version: No interrupts- getDepth in loop, updated from tests, added in absolute values, updated constants

// include libraries
#include <Adafruit_GPS.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include "dcawsDefines.h"

// declare GPS
Adafruit_GPS GPS(&GPS_Serial);

// declare servo
Servo servo;

//declare variables
File dcawsLog;                        // Create file on the sd card to log depth
bool missionReady = true;                // Create bool to track mission status through diagnostics
bool initLog = true;
bool newGPS = false;
int goodGPSCount = 0;
double depth;                            // variable to hold depth                                  
double current;                          // variable for internal current
double temperature;                      // variable for internal temp
double pressure;                         // variable for internal pressure
double leakVoltage;
double tSense3Voltage;                 //Voltage cell 3. 
double tSense2Voltage;                 //Voltage of cell 2.
double tSenseTopVoltage;
String dataString = "";
String errorString = "";
bool lowPow = false;
double avgDepth;                         // variable for avg Depth after going through moving average
bool avgInitZ = true;                    // declare initial mvavg flag for depth true
double zBuffer[WINSZ_Z];
double pidDepth;                         // will use if want avgDepth updated at 10hz
bool avgInitGPS = true;
double latBuffer[WINSZ_GPS], lonBuffer[WINSZ_GPS];
bool initGPS = true;
double avgLat;
double avgLon;
int targetCount = 1;
int targetDepth;
float td1;
float td2;
float td3;
float targetDepth1;
float targetDepth2;
float targetDepth3;
float pidError = 0;
float Fb = (M*g) + FBPOS;               // calculate total buoyancy force of system
float thrust;
float prevError = 0;
bool initDepthHold = true;
bool initSample = true;
float prevDepth = 0;
int surfaceCount = 0;
bool initPID = true;

// declare state machine variables
enum {GPS_DRIFT, ASCENT, SAMPLE_MISSION, GPS_FINAL, ABORT} state;


//create timers for mission
elapsedMillis sinceStart;
//create timer for PID control
elapsedMillis sincePrev;
//create timer for data renewal
elapsedMillis sinceDataFreq;
elapsedMillis sinceGPS;
elapsedMillis sinceErrorInTol;
elapsedMillis sinceTrigger;
//declare elapsed timer to use for gpsTimeout
elapsedMillis gpsTimeout;


void setup()
{
  char go = 'n';
  //initialize components
  setupRadio();
  setupTapSense();
  setupThruster();
  setupGPS();
  setupSD();
  setupSolenoids();
  delay(SETUP_DELAY);
  
  //prior to deployment conduct system Diagnosis and wait for user command to continue
  while(go != 'y')
  { 
    //Retrieve user input depths and run a system diagnostic
    getTargetDepths(); 
    systemDiagnosis();
    radio.print(F("Send 'y' (yes) to begin mission "));
    radio.println(F("or enter any other key to rerun diagnostic."));
    while(radio.available())
    {
      radio.read();
    }
    while(!radio.available())
    {
    } 
    go = (char)radio.read();   
  }
  // change to GPS Drift state
  state = SAMPLE_MISSION; //GPS_DRIFT;
}

void loop()
{
  if(!lowPow)
  {
    //pause interrupts and make copy of current depth reading
    getDepth();
    // average pressure (not sure if moving avg right)
    mvavgDepth(depth, avgInitZ);
    avgInitZ = false;
    // update depth log (and set depth for pid) at rate of 10Hz
    dataString = "";
    dataString = String(state)+ "," + String(depth)+ "," + String(avgDepth);
    dataString += "," + String(thrust);
    if(state == GPS_DRIFT || state == GPS_FINAL)
      dataString += "," + String(GPS.latitude)+ "," + String(GPS.longitude)+ "," + String(avgLat)+ "," + String(avgLon);
    if(state == ABORT)
      dataString += "," + errorString;
    if (sinceDataFreq >= DATA_HZ)
    {
      logData();
      if(state == GPS_FINAL)
        dcawsLog.close();
      pidDepth = avgDepth;
      checkSafetySensors();
      sinceDataFreq = 0;
    }
  }
  switch (state)
  {
    case GPS_DRIFT:
      if (sinceStart < GPS_DRIFT_TIME || initGPS)
      {
        getGPS();
        if (newGPS)
        {
          //radio.println("got new gps");
          // Calculate the moving average and set the init flag to false
          mvavgGPS((double) GPS.latitude, (double) GPS.longitude, avgInitGPS);
          avgInitGPS = false;
          if (initGPS)
          {
            sendGPS();
            initGPS = false;
            sinceStart = 0;
          }
          newGPS = false;
        }
      }
      else if ((sinceStart > GPS_DRIFT_TIME && initGPS) || (goodGPSCount < MIN_GPS_NUM))
      {
          // abort if times out before getting first value or enough good values
          errorString += "GPS timed out or recieved too few usable signals";
          state = ABORT;
      }
      else
      {
        getGPS();
        if (newGPS)
        {
          sendGPS();
          state = SAMPLE_MISSION;
        }
        checkSafetySensors();
      }
    break;

    case SAMPLE_MISSION:
      setTargetDepth(targetCount);
      calculatePID();
      sendPIDSignal();
      if (initDepthHold)
      {
        if ((abs(prevError) <= HOLD_TOL) && (abs(pidError) <= HOLD_TOL))
        {
          sinceErrorInTol = 0;
          initDepthHold = false;
        }
      }
      else if (sinceErrorInTol >= HOLD_TIME)
      {
          takeSample(targetCount);
      }
      if (targetCount > 3)
        state = ASCENT;
      break;

    case ASCENT:
      ascend();
      if (pidDepth <= 2 && prevDepth <= 2)
      {
        surfaceCount++;
        if (surfaceCount > SURF_NUM)
        {
          servo.writeMicroseconds(STOP_SIGNAL);
          state = GPS_FINAL;
          initGPS = true;
        }
      }
    break;

    case GPS_FINAL:
      getGPS();
      if (newGPS)
      {
        if (initGPS)
        {
          sinceGPS = 0;
          initGPS = false;
        }
        if (sinceGPS > GPS_SEND_FREQ)
        {
          sendGPS();
          sinceGPS = 0;
        }
      }
    break;

    case ABORT:
      if(tapSenseMonitor())
       {
         //turn off nonessentials
         lowPow = true;
         //turn off servo
         servo.writeMicroseconds(STOP_SIGNAL);
         //stop logging? (log low power, then somehow permanently close)
         logData();
         dcawsLog.close();
         // send to GPS_FINAL state to try and send GPS if enough pow
         state = GPS_FINAL;
       }
      else
      {
        //otherwise force log error and jump to ascent state
        logData();
        state = ASCENT;
      }
    break;
  }
}


