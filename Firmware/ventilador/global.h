#ifndef GLOBAL_H
#define GLOBAL_H

#define TIMETOBEEPINTHECFGSCREEN_MS 5000

/////////////////////////////////////////////
//Some general configurations:
//Serial print some debug information
#define DEBUG_CODE
#ifdef DEBUG_CODE
  //#define DEBUG_CODE_PULSES
#endif


//Defines used to calculate the total number of steps per revolution
#define MOTOR_STEPS_PER_REVOLUTION 200
#define SUB_DIVIDE_STEPS 4
#define REDUCTION_GEAR 2//((float)3.33f)
//Total number of steps per revolution
#define STEPS_PER_REVOLUTION (MOTOR_STEPS_PER_REVOLUTION*SUB_DIVIDE_STEPS*REDUCTION_GEAR) //steps per revolution

// Use the PWM to control the motor steps?
#define MOTOR_PWM

//Used only when the PWM is not present
#ifndef MOTOR_PWM
  #define MIN_PULSE_WIDTH 30 //minimun width in microseconds of the step pulse, related to either or not we use 1/2 step, 1/4 step, and so on.
#else
  #define MAX_RANGE_PWM_VALUE 40 //define the diference in the PWN configuration for the start speed and max speed
#endif

//should the expiration phase have a fixed speed?
//0 = no, otherwise use the speed value
//no that if the specified value is lower than the minimum speed
//required to obtain the desired I:E ratio, the this setting will be ignored
#define FIXED_EXPIRATION_SPEED 0//(STEPS_PER_REVOLUTION*2)//0

//Rotational direction:
// 1 means opening with clockwise rotation
// -1 means closing with clockwise rotation
#define ROTATIONAL_DIRECTION -1

// If MOTOR_ACCELERATION is zero, de motor will perform a fixed speed
// Otherwise, it will use acceleration. Note that acceleration mitigates steps loss
// MOTOR_ACCELERATION = 0 for fixed speed, MOTOR_ACCELERATION = between 3 an 20 for a fraction of the total time, where time/x represents the percentage of the time that the motor will use to accelerate
#define MOTOR_ACCELERATION 10

#if MOTOR_ACCELERATION > 0
  #ifdef MOTOR_PWM
    //for PWM, should we accelerate every pulse?
    //Our tests show that it obtains better results.
    #define ACCELERATE_EVERY_PULSE
  #endif
#endif

//If there is a start limit switch, uncomment the line bellow.
#define START_LIMIT_SWITCH
#ifndef START_LIMIT_SWITCH
  #define TOTAL_NUMBER_STEPS STEPS_PER_REVOLUTION // value obtained empirically
#endif

// Fraction of the total number of steps that is used to void the engine to reach the limit switchs every cicle
#define FRAC_SWITCH_LIMIT_OFFSET 40
/////////////////////////////////////////////


typedef enum
{
  NA = -1,
	S0,
	S1,
	S2,
	S3,
	S4
} State;

typedef struct
{
  long endLimitSwitchPos;
  int activeConfigPos;
} StartConfig;

//converted parameters
typedef struct {
  long stepPosIns;
  long stepPosExp;
  float insVel;
  float expVel;
#ifndef MOTOR_PWM
  float insAcel;
  float expAcel;
#else
  //double timeFreqRatio;
  unsigned char insStartSpeedFreq;
  unsigned char insTargetSpeedFreq;
  unsigned char insPWMPrescaler;
  unsigned char expStartSpeedFreq;
  unsigned char expTargetSpeedFreq;
  unsigned char expPWMPrescaler;
  #ifndef ACCELERATE_EVERY_PULSE
    unsigned long insMicroTime;
    unsigned long expMicroTime;
    unsigned long insMicroAcelTime;
    unsigned long expMicroAcelTime;
  #endif
#endif
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
#define POSMAX 15
#define POSMIN 1

//Limit switches defines
#define LIMIT_SWITCH_END_PIN 3
#define LIMIT_SWITCH_INI_PIN 2

//Motor relative defines
#define STEP_PIN 11 //so that it is also compatible with PWM
#define DIR_PIN 13
#define MAX_MOTOR_SPEED (STEPS_PER_REVOLUTION * 4) //maximum speed in steps per second, used for fixed speed
#define MAXIMUM_START_SEQ_NUM_STEPS (STEPS_PER_REVOLUTION*3) // used to in the start sequence
#define CONFIG_MOTOR_SPEED STEPS_PER_REVOLUTION // steps per second, used to in the start sequence



//Default configuration defines4
#define DEFAULT_POS_EXP 15
#define DEFAULT_VOLUME 100
#define DEFAULT_BPM 20
#define DEFAULT_IE_RATIO 2


// Clockwise = opening = positive
// Anticlockwise = closing = negative
void reportErr(char *erro);


State funcS0(State oldState);
State funcS1(State oldState);
State funcS2(State oldState);
State funcS3(State oldState);
State funcS4(State oldState);
void calculateMotorParam();
void configureInspMotor();
void configureExpMotor();
State verifyIHM(State oldState, InsExpSign phase);

extern State gState;
extern StartConfig gStartConfig;
extern MotorParameters gMotorParameters;
extern unsigned long gInitialBreathTime;

#endif
