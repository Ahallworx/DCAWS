// Alex Barker and Adam Hall
// 2/22/2019
// Code to run autonomous water sampler DCAWS
// Changes from previous version: additionalwork on system checks

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
//#define GPS_TXD 7                                 // Covered by Serial3
//#define GPS_RXD 8                                 // Covered by Serial3
//#define RADIO_TX 9                                // Covered by Serial2
//#define RADIO_RX 10                               // Covered by Serial2
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
#define HOLD_TOL .2                                   // declare tolerance on hold
#define g 9.81                                        // (gravity m/s^2)
#define RHO 1025                                      // density of seawater (kg/m^3)
#define Vs 5                                          // voltage supplied to depth sensor (V)
#define Vt 3                                          // voltage value input into teensy (V)
#define M 10.5                                        // Mass of system (kg)
#define A .09                                         // Area of attack of system (m^2)
#define CD .38                                        //the drag coefficient of system from solidworks
#define FBPOS 5                                       // positive buoyancy of system
#define PID_RANGE -2                                  // set PID to kick in when within 2 m of target depth
#define CONST_THRUST 6 ////////////////////////////////// need to calculate thrust(N) for desired constant velocity
#define MIN_GPS_NUM 10                                // Minimum number of good gps readings taken during GPS_DRIFT to consider successful
#define GPS_DRIFT_TIME 60000                          // desired duration of drift state in milliseconds
#define MAX_GPS_TIME 120000

//declare global variables 
double depth;
int targetCount = 1;
double depthCopy;
float signalESC;
int surfaceCount = 0;
double avgdepth;
float z_dot;
float z_doubledot;
//char key;
int goodGPSCount = 0;
int missionReady = 1;
//declare booleans for initializations
bool initDepthHold = true;
bool initSample = true;
bool sampleTaken = false;
bool initGPS = true;
bool newGPS = false;


//declare files
File DCAWS_Depth;
File DCAWS_GPS;

// Declare the moving average window size, latitude/longitude buffers and initialization flag
const int winsz = 10;
double zbuffer[winsz];
bool avginit = true;

// declare PID variables
float targetDepth;
float targetDepth_1 = 10;
float targetDepth_2 = 15;
float targetDepth_3 = 20;
float PID_error = 0;
float prev_error = 0;
float prev_depth = 0;

// declare and initialize PID variables
float Fb = (M*g) + FBPOS; // calculate total buoyancy force of system
float Kp = 0;//2.6
float Ki = 0;//0.225
float Kd = 0;//10
float thrust;
float PID_value = 0;
float PID_p = 0, PID_i = 0, PID_d = 0;

// declare GPS mvavg variables
const int winsz_GPS = 100;
double latbuffer[winsz_GPS], lonbuffer[winsz];
bool avginit_GPS = true;
double avglat, avglon;

// declare state machine variables
enum {GPS_DRIFT, ASCENT, SAMPLE_MISSION, GPS_FINAL, ABORT} state;

// declare timer
IntervalTimer timerDepth;

//declare elapsed timer to use for timeouts
elapsedMillis timeout;

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
  while(go != 'y' || go != )
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
  depthCopy = depth;
  interrupts();
  // average pressure (not sure if moving avg right)

  // update depth log (and set depth for pid) at rate of 10Hz
  if (sinceDataFreq >= 100)
  {
    logDepth();
    //pidDepth = avgdepth;
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
          mvavg_GPS((double) GPS.latitude, (double) GPS.longitude, avginit_GPS, &avglat, &avglon);
          avginit_GPS = false;
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
        //if(checkSystems())
        state = SAMPLE_MISSION;
      }
    break;

    case SAMPLE_MISSION:
      setTargetDepth(targetCount);
      calculatePID();
      sendPIDSignal();
      if (initDepthHold)
      {
        if (prev_error <= HOLD_TOL && PID_error <= HOLD_TOL)
        {
          sinceErrorInTol = 0;
          initDepthHold = false;
        }
      }
      if (sinceErrorInTol >= 10000)
      {
        if (!sampleTaken)
        {
          takeSample(targetCount);
        }
      }
      prev_error = PID_error;
      prev_depth = avgdepth/*pidDepth*/;
      if (targetCount > 3)
        state = ASCENT;
      break;

    case ASCENT:
      ascend();
      if (depthCopy <= 2 && prev_depth <= 2)
      {
        surfaceCount++;
        if (surfaceCount > 10)
        {
          servo.writeMicroseconds(1500);
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
        if (sinceGPS > 50000)
        {
          sendGPS();
          sinceGPS = 0;
        }
      }
    break;

    case ABORT:
    break;
  }
}


