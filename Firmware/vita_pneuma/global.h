#ifndef GLOBAL_H
#define GLOBAL_H

#include "parameters.h"
#include "alarm.h"
#include "pressure.h"


#ifdef VOLUME_USE_ML
	#define VOLUME_CALC(X_VAL) ((int)(VOLUME_PULSE_TO_ML_COEF_A*(X_VAL)+VOLUME_PULSE_TO_ML_COEF_B))
#else
	#define VOLUME_CALC(X_VAL) ((int)X_VAL)
#endif
//necessary no matter if VOLUME_USE_ML is set (0.99f is used to ceil the value)
#define VOLUME_INV_CALC(X_VAL) ((int)((0.99f+X_VAL-VOLUME_PULSE_TO_ML_COEF_B)/((float)VOLUME_PULSE_TO_ML_COEF_A)))

/////////////////////////////////////////////
//Some general configurations:

#if DEBUG_CODE != 0
  class SendOnlySoftwareSerial;
  extern SendOnlySoftwareSerial txSerial;
#endif

//Defines used to calculate the total number of steps per revolution
#define MOTOR_STEPS_PER_REVOLUTION 200 
#define SUB_DIVIDE_STEPS 4
#define REDUCTION_GEAR 2//((float)3.33f)
#define STEPS_PER_REVOLUTION (MOTOR_STEPS_PER_REVOLUTION*SUB_DIVIDE_STEPS*REDUCTION_GEAR) //steps per revolution

// Use the PWM to control the motor steps?
#define MOTOR_PWM

//Used only when the PWM is not present
#ifndef MOTOR_PWM
  #define MIN_PULSE_WIDTH 30 //minimun width in microseconds of the step pulse, related to either or not we use 1/2 step, 1/4 step, and so on.
#else
  #define MAX_RANGE_PWM_VALUE 45 //define the diference in the PWN configuration for the start speed and max speed
#endif





//this value is multiplied by the calculated speed
//1 = takes full expiration time to reach the final position, > 1 means faster
#define SET_EXPIRATION_SPEED 2//WARNING: if SET_EXPIRATION_SPEED is changed, one might have to increase the watchdog reset time since breath and plateau procedures do not call watchdogreset.
#if SET_EXPIRATION_SPEED <= 1
#error "SET_EXPIRATION_SPEED <= 1 does not allow the device to perform all the required procedures."
#endif


// If MOTOR_ACCELERATION is zero, de motor will perform a fixed speed
// Otherwise, it will use acceleration. Note that acceleration mitigates steps loss
// MOTOR_ACCELERATION = 0 for fixed speed, MOTOR_ACCELERATION = between 3 an 20 for a fraction of the total time, where time/x represents the percentage of the time that the motor will use to accelerate
#define MOTOR_ACCELERATION 5

#if MOTOR_ACCELERATION > 0
  #ifdef MOTOR_PWM
    //for PWM, should we accelerate every pulse?
    //Our tests show that it obtains better results.
    #define ACCELERATE_EVERY_PULSE
  #endif
#endif

// Base on MM_SWITCH_LIMIT_OFFSET, we obtain the number of steps

//#define PINION_DIAMATER 28 //mm
#define PINION_DIAMATER 16.486f //mm
#define PIf 3.14159265f
//Used to convert mm to step
#define STEPS_PER_MM ((float)STEPS_PER_REVOLUTION/(PIf*PINION_DIAMATER))
//Used to convert step to mm
#define MM_PER_STEPS ((float)(PIf*PINION_DIAMATER)/STEPS_PER_REVOLUTION)
/*This is not a good aproach.
//MM_TEST_CYCLE_INCREASE must be greater than MM_SWITCH_LIMIT_OFFSET
#if MM_TEST_CYCLE_INCREASE < (MM_SWITCH_LIMIT_OFFSET + 1)
	#undef MM_TEST_CYCLE_INCREASE
	#define MM_TEST_CYCLE_INCREASE (MM_SWITCH_LIMIT_OFFSET + 1)
#endif*/
//Convert the MM_SWITCH_LIMIT_OFFSET to steps
#define STEPS_SWITCH_LIMIT_OFFSET (MM_SWITCH_LIMIT_OFFSET*STEPS_PER_MM)
//How many extra steps will be performed to guarantee that the expiration limit switch will be performed?
#define STEPS_TEST_CYCLE_INCREASE ((float)MM_TEST_CYCLE_INCREASE*STEPS_PER_MM)


//in order to allow different values fot inspiration and expiration offset:
/*
#if MM_EXPIRATION_POSITION_OFFSET > MM_SWITCH_LIMIT_OFFSET
	//Convert the MM_EXPIRATION_POSITION_OFFSET to steps
	#define STEPS_EXPIRATION_POSITION_OFFSET (MM_EXPIRATION_POSITION_OFFSET*STEPS_PER_MM)
#else
	//otherwise, it is simply equal to STEPS_SWITCH_LIMIT_OFFSET
	#define STEPS_EXPIRATION_POSITION_OFFSET STEPS_SWITCH_LIMIT_OFFSET
#endif
*/

/////////////////////////////////////////////



typedef enum
{
  NA = -1,
	S0,
	S1,
	S2, // Must be in crescent order, from S0 to S4.
	S3,
	S4
} State;


typedef enum {
  NORMAL_CYCLE = 0,
  TEST_CYCLE = 1,
  NUMB_CYCLES
}CycleType;

typedef struct
{
  int stepPosIns;
  int stepPosExp;
  int posExpVol; //start exp. position used for volume calculation
  float insVel;
  float expVel;
#ifndef MOTOR_PWM
  float insAcel;
  float expAcel;
#else
  //double timeFreqRatio;
  unsigned int insStartSpeedFreq;
  unsigned int insTargetSpeedFreq;
  unsigned char insPWMPrescaler;
  unsigned int expStartSpeedFreq;
  unsigned int expTargetSpeedFreq;
  unsigned char expPWMPrescaler;
  #ifndef ACCELERATE_EVERY_PULSE
    unsigned long insMicroTime;
    unsigned long expMicroTime;
    unsigned long insMicroAcelTime;
    unsigned long expMicroAcelTime;
  #endif
#endif
} CycleParameters;

//converted parameters
typedef struct {
  CycleParameters cycles[NUMB_CYCLES];
} MotorParameters;


typedef enum
{
  INSPIRATION_SIGN = -1,
  EXPIRATION_SIGN = 1
} InsExpSign;


typedef enum
{
  ANTICLOCKWISE_SIGN = -1,
  CLOCKWISE_SIGN = 1
} ClockSign;

//Position of the Insp and Exp Phases
//#define MAX_POS_EXP 15
//#define MIN_POS_EXP 1

//Limit switches defines
#define LIMIT_SWITCH_END_PIN 9
#define LIMIT_SWITCH_INI_PIN 2

//Motor relative defines
#define DIR_PIN 10
#define STEP_PIN 3 // PIN 3 is the PWM output OC2B
//#define STEP_PIN 11
//#define DIR_PIN 13

#if STEP_PIN != 3 && defined(MOTOR_PWM)
  #error "While using MOTOR_PWM, STEP_PIN must be set to pin 3."
#endif

#define MAX_MOTOR_SPEED (STEPS_PER_REVOLUTION * 4) //maximum speed in steps per second, used for fixed speed
#define MAXIMUM_START_SEQ_NUM_STEPS (STEPS_PER_REVOLUTION*3) // used to in the start sequence
#define CONFIG_MOTOR_SPEED 533 //533 ~ 10BPM for 2x reduction and 1/4 step. //STEPS_PER_REVOLUTION steps per second, used to in the start sequence


typedef enum {
  RESTART_UNEXPECTED,
  RESTART_NORMAL,
  RESTART_WATCHDOG,
  TOTAL_RESTART_METHODS
} RestartMethod;

//Default configuration defines4
//#define DEFAULT_POS_EXP 15
//#define MIN_POS_EXP 1
//#define MAX_POS_EXP 15
#define DEFAULT_VOLUME VOLUME_INV_CALC(600) //pulses
#define MIN_VOLUME VOLUME_INV_CALC(200)
#define MAX_VOLUME VOLUME_INV_CALC(600)
#define DEFAULT_BPM 20
#define MIN_BPM 10
#define MAX_BPM 30
#define DEFAULT_IE_RATIO 2
#define MIN_IE_RATIO 1
#define MAX_IE_RATIO 3
#define DEFAULT_PIP 23
#define MIN_PIP 0
#define MAX_PIP 35
#define INC_PIP 1
#define DEFAULT_PEEP_MIN 4
#define MIN_PEEP_MIN -5
#define MAX_PEEP_MIN 20
#define INC_PEEP_MIN 1
#define DEFAULT_TRIGGER 1.2
#define MIN_TRIGGER 0.5f
#define MAX_TRIGGER 3.0f
#define INC_TRIGGER 0.1f
#define DEFAULT_OP_MODE AC_MODE
#define DEFAULT_RESTART_METHOD RESTART_UNEXPECTED
#define DEFAULT_NUMBER_STEPS STEPS_PER_REVOLUTION // value obtained empirically
#define DEFAULT_SENSOR_C1 ((float)(KPA2CMH2O*ADCVREF/1023)/PSENSORGAIN) //initial C1 value
#define DEFAULT_SENSOR_C2 ((float)(KPA2CMH2O*PSENSOROFFSET)/PSENSORGAIN) //initial C2 value



enum {
  LIMIT_SWITCH_REACHED = 0,
  LIMIT_SWITCH_NOT_REACHED = 1
};


typedef enum {
  AC_MODE,
  C_MODE,
  N_MODE,
  TOTAL_MODES
} OpMode;


// We use this struct to share info between the IHM
// and others part of the code
typedef struct RespiratorInfo{
  int volume_pulses;
  //char pos_exp;
  char bpm;
  float ie_ratio;
  char peep_min;
  char pip_max;
  float trigger;
  char op_mode;
  long endLimitSwitchPos;
  float sensorCalibC1;
  float sensorCalibC2;
  RestartMethod restartMethod;
  bool faultDetected;
} RespiratorInfo;




struct AlarmInfo;

typedef struct RespiratorEstInfo{
  int volume_est;
  char bpm_mea;
  float ie_ratio_mea;
  char cycle_bpm; //used to ensure that we know the current cycle BPM, even if the value is reset.
  int currentAlarms;
  AlarmInfo alarms;
  bool standBy;
  bool assist_cur_breath; //0 for controled, 1 for assistied current breath
  float flux_est;   //current flux
  unsigned char currentCycle;
  bool newMotorConfig;
#ifdef USE_PRESSURE_SENSOR
  float pip_max_mea;
  float peep_min_mea;
  float maxCyclePressure; //used to determine the pip
#endif
} RespiratorEstInfo;


extern RespiratorInfo gActiveRespCfg;
extern RespiratorEstInfo gActiveRespEst;

#define ROUND_INT(X_VAL) (X_VAL >= 0? ((int)(X_VAL+0.5f)) : ((int)(X_VAL-0.5f)))
#define ROUND_CHAR(X_VAL) (X_VAL >= 0? ((char)(X_VAL+0.5f)) : ((char)(X_VAL-0.5f)))
#define ROUND_DOT5_FLOAT(X_VAL) ((X_VAL >= 0? ((int)((X_VAL*2)+0.5f)) : ((int)((X_VAL*2)-0.5f)))/2.0f)

State funcS0();
State funcS1();
State funcS2();
State funcS3(State oldState);
State funcS4(State oldState);
void calculateMotorParam(bool forceUpdate);
State verifyIHM(State oldState);//, InsExpSign phase);
bool goToPositionAndFindSwitch(int endSwitch, long finalPosition, float *maxPressure = 0);
bool moveStepsAndFindSwitch(int endSwitch, long numSteps, float *maxPressure = 0);
void performStartProcedure(bool hiddenConfig, int totalNumberTests);
void calcSampleMeanStdDevError(long *samples, int size, float *mean, float *sampleStdDev, float *error);
CycleParameters *configureExpMotor(bool goToEndLimitSwitch);

#ifdef USE_PRESSURE_SENSOR
	void updateMaxPressure();
#endif

void systemFail(int msg);
State getSystemState();
extern MotorParameters gMotorParameters;
extern unsigned long gInitialBreathTime;
#endif
