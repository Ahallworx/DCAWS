// Alex Barker and Adam Hall
// 3/22/2019
// Code to run autonomous water sampler DCAWS
// Changes from previous version: updated from tests, solenoid closing in setup, solenoid closing issue

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
bool initCheckGPS = true;
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
double depthCopy;
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
int targetDepth1 = 10 + OFFSET;
int targetDepth2 = 15 + OFFSET;
int targetDepth3 = 20 + OFFSET;
float pidError = 0;
float Fb = (M*g) + FBPOS;               // calculate total buoyancy force of system
float thrust;
float prevError = 0;
bool initDepthHold = true;
bool initSample = true;
float prevDepth = 0;
int surfaceCount = 0;

// declare state machine variables
enum {GPS_DRIFT, ASCENT, SAMPLE_MISSION, GPS_FINAL, ABORT} state;

// declare timer
IntervalTimer timerDepth;

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
    //run a system diagnostic 
    systemDiagnosis();
    radio.print(F("Send 'y' (yes) to begin mission "));
    radio.println(F("or enter any other key to rerun diagnostic."));
    while(!radio.available())
    {
    } 
    go = (char)radio.read();   
  }
  // change to GPS Drift state
  state = GPS_DRIFT;
  // begin depth interval timer
  timerDepth.begin(getDepth, 10000);

}

//create timers for mission
elapsedMillis sinceStart;
//create timer for PID control
elapsedMillis sincePrev;
//create timer for data renewal
elapsedMillis sinceDataFreq;
elapsedMillis sinceGPS;
elapsedMillis sinceErrorInTol;
elapsedMillis sinceTrigger;

void loop()
{
  if(!lowPow)
  {
    //pause interrupts and make copy of current depth reading
    noInterrupts();
    depthCopy = depth;
    interrupts();
    // average pressure (not sure if moving avg right)
    mvavgDepth(depthCopy, avgInitZ);
    avgInitZ = false;
    // update depth log (and set depth for pid) at rate of 10Hz
    dataString = "";
    dataString = String(sinceDataFreq)+ "," + String(depthCopy)+ "," + String(avgDepth);
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
      if (sinceStart < GPS_DRIFT_TIME)
      {
        getGPS();
        if (newGPS)
        {
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
      else if (sinceStart > GPS_DRIFT_TIME && (initGPS || (goodGPSCount < MIN_GPS_NUM)))
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
        if (prevError <= HOLD_TOL && pidError <= HOLD_TOL)
        {
          sinceErrorInTol = 0;
          initDepthHold = false;
        }
      }
      else if (sinceErrorInTol >= HOLD_TIME)
      {
          takeSample(targetCount);
      }
      prevError = pidError;
      prevDepth = pidDepth;
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
      if (GPS.fix)
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
         noInterrupts();
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


