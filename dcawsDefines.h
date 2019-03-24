

// declare serial constants
#define GPS_Serial Serial3
#define radio Serial2

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
#define MIN_SAFE_CURR  0.0                   // Minimum safe current before abort is neccesary
#define MAX_SAFE_CURRENT 20.0
#define MAX_SAFE_TEMP 85.0               // Maximum operating temp before abort needed
#define V_SUPPLY 5.0                     // 5 volt supply voltage
//#define MAX_SAFE_PRESS                    // Maximum safe pressure before aborting
# define MIN_SAFE_LEAK_V 3.0             // Minimum voltage of 3V n leak sensor any less means leak
# define MIN_SAFE_TSENSE 3.5
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FBPOS 4.35                          // positive buoyancy of system
#define A .09                            // Area of attack of system (m^2)
#define CD .38                           //the drag coefficient of system from solidworks
#define M 10.5                           // Mass of system (kg)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define T_GAIN 39.84063745
#define MIN_PWM 1525
#define MAX_PWM 1900
#define HOLD_TIME  10000                 //hold time of ten seconds for depth and sample
#define SURF_NUM 10                      //number of times for pressure sensor to read surface value before claiming surface
#define GPS_SEND_FREQ 50000


