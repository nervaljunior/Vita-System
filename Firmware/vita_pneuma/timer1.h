#ifndef TIMER1_H_
#define TIMER1_H_

#include "Arduino.h"
#include "pressure.h"

void timer1_init();

#ifdef USE_PRESSURE_SENSOR

typedef enum{
	TRIGGERNONE,
	TRIGGERINSPIRATION,
	TRIGGERFAULT,
} PSensorTrigger;


typedef enum{
  NO_INFO = -1,
	PEEPVALID = 0, // A valid PEEP was calculated
	PEEPVOLATILE, // The pressure in the PEEP period changes too much
	PEEPNOTREADY, // Not enough samples were captured
	PEEPNEWSAMPLE // A new sample was captured, but the PEEP was not calculated yet
} PeepInfo;


#define ADCVALUETOCMH2O(adcvalue, c1, c2) (((float)(adcvalue)*c1) - c2)

//#define CMH2OTOADCVALUE(cmH2O, c1, c2) ((unsigned int) ((cmH2O)*ONEDIVBYC1+C2DIVBYC1+0.5))
#define CMH2OTOADCVALUE(cmH2O, c1, c2) ((unsigned int) ((cmH2O)*(1/c1)+(c2/c1)+0.5))


void timer1_pSensor_sampling_start();

void timer1_pSensor_sampling_stop();

float timer1_pSensor_raw_start_value(int numSamples);

float timer1_pSensor_read_cmH2O();

float timer1_pSensor_getPeep();

void set_calib_value(float pressureAngCoefC1, float pressureLinCoefC2);
bool timer1_pSensor_newSampleAvailable();

int copyGraphBuffer(char *destBuffer);

PSensorTrigger timer1_pSensor_trigger(float triggerFallBelowPeep_cmH2O);

PeepInfo timer1_pSensor_calcPeep(bool firstTimeOfThisCycle, float * newSample);

#endif

bool timer1_beepIsRunning();

void timer1_beep_start();

void timer1_beep_stop();

void timer1_beep_update();

bool timer1_beepIsRunning();

#endif // TIMER1_H_
