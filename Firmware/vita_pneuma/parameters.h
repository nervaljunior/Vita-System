#ifndef PARAMETERS_H
#define PARAMETERS_H

// Are we debugging?
// 0: No Debug
// 1: Normal Debug
// 2: Debug PLOT PRESSURE SAMPLES
// 3: Debug ISR PRESSURE SAMPLING EXECTIME
// 4: Debug ISR PWM
// 5: Debug motor calibration procedure
// 6: Motor acceleration debug
// 7: Motor position Debug
// 8: PEEP/PIP plots
#define DEBUG_CODE 0

// Do we have a pressure sensor? Uncomment the line below for "yes".
//#define USE_PRESSURE_SENSOR

// The distance that is used to void the engine to reach the limit switch every cycle
#define MM_SWITCH_LIMIT_OFFSET 1 //mm

// The initial position offset from the expiration limit switch.
// Must be >= MM_SWITCH_LIMIT_OFFSET.
#define STEPS_EXPIRATION_POSITION_OFFSET 50 //250 //50

// The initial position offset for the test cycle.
// Should be < STEPS_EXPIRATION_POSITION_OFFSET
#define STEPS_TEST_CYCLE_POSITION_OFFSET 20

//How many mm should we go below the expiration limit switch to guarantee that it will be activated in the test cycle?
#define MM_TEST_CYCLE_INCREASE 1 //should not be < MM_SWITCH_LIMIT_OFFSET


// If there is a start limit switch? Uncomment the line below for "yes".
#define START_LIMIT_SWITCH

// Are we using the volume in ml? Uncomment the line below for "yes".
#define VOLUME_USE_ML

//If the VOLUME_USE_ML is used, the coef. A and B.
#define VOLUME_PULSE_TO_ML_COEF_A 0.77945f
#define VOLUME_PULSE_TO_ML_COEF_B -281.82586f
//#define VOLUME_PULSE_TO_ML_COEF_A 0.658848f
//#define VOLUME_PULSE_TO_ML_COEF_B -221.266f

// What is the maximum fraction allowable in the test procedures?
#define MAX_ALLOWABLE_STEP_ERROR 0.02f //0.02 means 2%, which is about 28,53ml

// How often the test cycle must be performed?
#define CYCLES_PER_MOTOR_TEST 250 //Must be <= 255. For checking the expiration limit switch.
//While debugging, please note that TEST cycles WILL lead to an error at the I:E rate.

// Rotational direction: 1 or -1. Used to invert the motor rotation in exp/insp phases.
#define ROTATIONAL_DIRECTION 1

// PIP must be at least MINIMUM_PIP_INCREASE above PEEP.
#define MINIMUM_PIP_INCREASE -30.0f //Must be POSITIVE. (negative is just to avoid alarms for test purposes) TODO: Define the correct value

// Comparing to the mean historic PIP, the current PIP falling below MAXIMUM_ALLOWABLE_PIP_DROP cmH2o will generate an alarm
#define MAXIMUM_ALLOWABLE_PIP_DROP 30.0f //TODO: Define the correct value

// Comparing to the mean historic PEEP, the current PEEP falling below MAXIMUM_ALLOWABLE_PEEP_DROP cmH2o will generate an alarm
#define MAXIMUM_ALLOWABLE_PEEP_DROP 30.0f //TODO: Define the correct value

// Drop pressure value below the peep to start counting the time until the pressure drops below the value established in the trigger.
//#define TRIGGER_START_COUNTING_TIME_CMH2O -0.5 //not currently used

//The percentage used to calibrate the pressure sensor.
#define PRESSURE_CALIBRATION_VOLUME_PERC 0.8f //0.8f = 80% of the total number of steps between inspiration and expiration end limit swicthes

// Sampling period to measure PEEP in ms
#define PEEP_SAMPLINGTIME_MS 50


// If during the PEEP calculation, the pressure varies more than PEEP_THRESHOLD_VOLATILE_CMH2O, then a volatile PEEP alarm is generated. ALARM_D
#define PEEP_THRESHOLD_VOLATILE_CMH2O 3


//Number of motor tests in hidden config.
#define HIDDEN_CONFIG_NUM_TESTS 3

//If LCD show be reseted every key pressed
#define RESET_LCD_OFTEN

//Makes the stand-by button to enforce just 1 cycle every time the user press "Ok", stopping the motor every cycle.
//#define STAND_BY_DEBUG


#endif //PARAMETERS_H
