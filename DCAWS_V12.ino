// Alex Barker and Adam Hall
// 3/04/2019
// Code to run autonomous water sampler DCAWS
// Changes from previous version: verified safety check codes, variable overhaul.

// include libraries
#include <Adafruit_GPS.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>

// declare serial constants
#define GPS_Serial Serial3
#define radio Serial2

// declare GPS
Adafruit_GPS GPS(&GPS_Serial);

// declare servo
Servo servo;

// declare pin constants
#define ESC_RC_PW 2
#define CURR_SENSOR 14
#define TEMPERATURE_SENSOR 15
#define LEAK_SENSOR 16
#define INT_PRESS_SENSOR 17
#define IMU_SDA 18
#define IMU_SCL 19
#define SOLENOID_1 20
#define SOLENOID_2 21
#define SOLENOID_3 22
#define DEPTH_SENSOR 23
#define BATTERY_SENSOR_EN 31
#define BATTERY_CELL_READ 32
#define THIRD_CELL_READ 33
#define SECOND_CELL_READ 34
#define TOP_CELL_READ 35
#define chipSelect BUILTIN_SDCARD

//declare constants
#define STOP_SIGNAL 1500                  //PWM signal that stops thruster
#define SETUP_DELAY 1000                  // time delay for setup signals
#define MAX_GPS_TIME 120000               // 2min cutoff for GPS to recieve new signal
#define RES 1024.0                        // 10 bit resolution for adc conversion
#define V_TEENSY 3.0                      // 3 volt reference into the teensy
#define RHO 1025.0                        // density of seawater (kg/m^3)
#define g 9.81                            // (gravity m/s^2)
//#define MIN_SAFE_CURR                     // Minimum safe current before abort is neccesary
#define MAX_SAFE_TEMP 85.0               // Maximum operating temp before abort needed
#define V_SUPPLY 5.0                     // 5 volt supply voltage
//#define MAX_SAFE_PRESS                    // Maximum safe pressure before aborting
# define MIN_SAFE_LEAK_V 3.0             // Minimum voltage of 3V n leak sensor any less means leak
#define WINSZ_Z 10                       // window size used for depth of 10
#define DATA_HZ 100                      //update frequency of 10 HZ for log (and PID)
#define GPS_DRIFT_TIME 60000             // desired duration of drift state in milliseconds
#define WINSZ_GPS 100                    // window size used for GPS of 100
#define MIN_GPS_NUM  10                  // Minimum number of good gps readings taken during GPS_DRIFT to consider successful
#define PID_RANGE -2
#define HOLD_TOL .2                      // declare tolerance on hold
#define KP 2.6
#define KI .225
#define KD 10
#define A .09                            // Area of attack of system (m^2)
#define CD .38                           //the drag coefficient of system from solidworks
#define M 10.5                           // Mass of system (kg)
#define T_GAIN 17.98561151
#define HOLD_TIME  10000                 //hold time of ten seconds for depth and sample
#define SURF_NUM 10                      //number of times for pressure sensor to read surface value before claiming surface
#define GPS_SEND_FREQ 50000


//declare variables
File DCAWS_Depth;                        // Create file on the sd card to log depth
File DCAWS_GPS;                          // Create file on the sd card to log GPS
bool missionReady = true;                // Create bool to track mission status through diagnostics
bool newGPS = false;
int goodGPSCount = 0;
double depth;                            // variable to hold depth
double current;                          // variable for internal current
double temperature;                      // variable for internal temp
double pressure;                         // variable for internal pressure
double avgDepth;                         // variable for avg Depth after going through moving average
bool avgInitZ = true;                    // declare initial mvavg flag for depth true
//double pidDepth;                         // will use if want avgDepth updated at 10hz
bool avgInitGPS = true;
bool initGPS = true;
double avgLat;
double avgLon;
int targetCount = 1;
int targetDepth;
int targetDepth1 = 10;
int targetDepth2 = 15;
int targetDepth3 = 20;
float pidError = 0;
float thrust;
float prevError = 0;
bool initDepthHold = true;
bool sampleTaken = false;
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
  setupThruster();
  setupRadio();
  setupGPS();
  setupSD();
  setupSolenoids();
  
  //prior to deployment conduct system Diagnosis and wait for user command to continue
  while(go != 'y')
  { 
    //run a system diagnostic 
    systemDiagnosis();
    radio.print(F("Send 'y' (yes) to begin mission "));
    radio.print(F("or enter any other key to rerun diagnostic."));
    if(radio.available())
      {
        go = radio.read();
      }    
  }
  state = GPS_DRIFT;
  timerDepth.begin(getDepth, 10000);

}

//create timer for mission
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
  //pause interrupts and make copy of current depth reading
  noInterrupts();
  double depthCopy = depth;
  interrupts();
  // average pressure (not sure if moving avg right)
  mvavgDepth(depthCopy, avgInitZ);
  avgInitZ = false;
  // update depth log (and set depth for pid) at rate of 10Hz
  if (sinceDataFreq >= DATA_HZ)
  {
    logDepth();
    //pidDepth = avgDepth;
    checkSafetySensors();
    sinceDataFreq = 0;
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
          logGPS();
          newGPS = false;
        }
      }
      else if (sinceStart > GPS_DRIFT_TIME && (initGPS || (goodGPSCount < MIN_GPS_NUM)))
      {
          state = ABORT;
      }
      else
      {
        getGPS();
        if (newGPS)
          sendGPS();
        
        state = SAMPLE_MISSION;
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
      if (sinceErrorInTol >= HOLD_TIME)
      {
        if (!sampleTaken)
        {
          takeSample(targetCount);
        }
      }
      prevError = pidError;
      prevDepth = avgDepth/*pidDepth*/;
      if (targetCount > 3)
        state = ASCENT;
      break;

    case ASCENT:
      ascend();
      if (depthCopy <= 2 && prevDepth <= 2)
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
      /* if(power < MIN_SAFE_POW)
       * {
       *  //turn off nonessentials
       *  //turn off server
       * }
       * else
       * {
       *  ascend();
       * }
       */
    break;
  }
}


