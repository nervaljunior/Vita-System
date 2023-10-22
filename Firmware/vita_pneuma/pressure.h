#ifndef PRESSURE_H_
#define PRESSURE_H_

#include "parameters.h"




// Plateau period is necessary even if there is no pressure sensor
// to allow the system to verify the maximum pressure after inspiration procedure
#define PLATEAU_PERIOD_MS 2000
//Used to allow the motor to do not lose steps
#define END_INSP_DELAY_MS 10

#define TOTAL_MOTOR_WAIT_PERIOD (PLATEAU_PERIOD_MS+END_INSP_DELAY_MS)

//Maximum allowed wait time in terms of the total inspiration period
#define MAX_WAIT_PERIOD_FRAC_INSP 0.666f //fraction of insp. time


#ifdef USE_PRESSURE_SENSOR


	// TODO: check if this sampling time is suitable
	#define TRIGGER_SAMPLINGTIME_MS 20

	// TODO: check if this time is suitable for checking pacient attempt to inspire
	#define TRIGGER_PRESSURE_FALL_TIME_INSPIRATION_MS 500

	#define TRIGGER_MAXTIME_MS 500

#endif

//Defines used to calculate the initial C1 and C2 sensor parameters.

// VS that supplies pressure sensor
#define VS 5
// ADC vref
#define ADCVREF 1.1f

#define KPA2CMH2O 10.1972f

// Initial pressure sensor gain. For the MPX5050 it is 0.018xVS V/kPA
// Initial pressure sensor gain. For the MPX5100 it is 0.009xVS V/kPA
#define PSENSORGAIN (0.009f*VS)

// Initial pressure sensor output offset. For the MPX5050 it is 0.04xVS V
#define PSENSOROFFSET (0.04f*VS)

#define CALC_SENSOR_C1(x1,y1,x2,y2) ((y2-y1)/(x2-x1))
#define CALC_SENSOR_C2(x1,c1) (x1*c1)


#endif
