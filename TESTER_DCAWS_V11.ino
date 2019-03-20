// Alex Barker and Adam Hall
// 3/18/2019
// Code to run autonomous water sampler DCAWS
// Changes from previous version: changed logging style, finished abort state, added new T gain and depth offset,reinitialize sample mission bools so all three samples should work

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
#define OFFSET .6606                      // distance between pressure sensor and solenoid inlets
#define STOP_SIGNAL 1500                  // PWM signal that stops thruster
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
#define PID_RANGE -2                     // Range in m at which PID implements full control
#define HOLD_TOL .2                      // declare tolerance on hold
#define KP 2.6                           
#define KI .225
#define KD 10
#define FBPOS 5                          // positive buoyancy of system
#define A .09                            // Area of attack of system (m^2)
#define CD .38                           //the drag coefficient of system from solidworks
#define M 10.5                           // Mass of system (kg)
#define T_GAIN 39.84063745
#define MIN_PWM 1525
#define MAX_PWM 1900
#define HOLD_TIME  10000                 //hold time of ten seconds for depth and sample
#define SURF_NUM 10                      //number of times for pressure sensor to read surface value before claiming surface
#define GPS_SEND_FREQ 50000


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
String dataString = "";
String errorString = "";
bool lowPow = false;
double avgDepth;                         // variable for avg Depth after going through moving average
bool avgInitZ = true;                    // declare initial mvavg flag for depth true
double pidDepth;                         // will use if want avgDepth updated at 10hz
bool avgInitGPS = true;
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
    go = radio.read();   
  }
  // once yes has been sent
  // close solenoids
  digitalWrite(SOLENOID_1, LOW);
  digitalWrite(SOLENOID_2, LOW);
  digitalWrite(SOLENOID_3, LOW);
  // change to GPS Drift state
  state = GPS_DRIFT;
  // begin depth interval timer
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
  if(!lowPow)
  {
    //pause interrupts and make copy of current depth reading
    noInterrupts();
    double depthCopy = depth;
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
      //calculatePID();
      //sendPIDSignal();
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
      //prevError = pidError;
      //prevDepth = pidDepth;
      if (targetCount > 3)
        state = ASCENT;
      break;

    case ASCENT:
      ascend();
      if (avgDepth/*depthCopy*/ <= 2 && prevDepth <= 2)
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
       *  lowPow = true;
       *  //turn off servo
       *  servo.writeMicroseconds(STOP_SIGNAL);
       *  noInterrupts();
       *  //stop logging? (log low power, then somehow permanently close)
       *  logData(dataString);
       *  dcawsLog.close();
       *  // send to GPS_FINAL state to try and send GPS if enough pow
       *  state = GPS_FINAL;
       * }
       * else
       * {*/
            //otherwise force log error and jump to ascent state
            logData();
            state = ASCENT;
       /* }
       */
    break;
  }
}


