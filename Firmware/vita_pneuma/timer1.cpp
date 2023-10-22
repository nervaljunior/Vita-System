#include "timer1.h"
#include "alarm.h"
#include "global.h"
#include "pressure.h"
#include "ihm.h"
#include "watchdog.h"

#if DEBUG_CODE != 0 //only special codes
#include "SendOnlySoftwareSerial.h"
#endif

// PIN B4 is the pin 12 of the arduino
// These 3 macros should correspond to the same port and pin
#define BUZZER_ARDUINO_PIN 12
// This macro should be PINA, PINB, PINC, PIND, ...
#define BUZZER_MCU_PORTPIN PINB
// This macro should be one of the values 0,1,2,3,4,5,6,7
#define BUZZER_MCU_PIN 4

#define FREQ_OPTION_BUZZER 30

static bool gBeepIsRunnig = false;

#ifdef USE_PRESSURE_SENSOR

//TODO: test higher values and obtain lower sampling rates
#define FREQ_OPTION_PRESSURE_SENSOR 305 //52Hz. Please use multiples of ~25,6Hz, which is the update of the pressure used in the graph screen
#define FREQ_RESULT_PRESSURE_SENSOR (51.06f)

#define PRESSUREMOVINGAVERAGELENGTH 16
#define SHIFT_CTE_TO_DIV 4

// TODO: MOVE TO global.h
#define PIN_PRESSURE A0



//Initial values for C1 and C2. Note that these values must be configured
float gPressureAngCoefC1 = ((float)(KPA2CMH2O*ADCVREF/1023)/PSENSORGAIN);
float gPressureLinCoefC2 = ((float)(KPA2CMH2O*PSENSOROFFSET)/PSENSORGAIN); //offset


//#define ONEDIVBYC1 ((float)(1/gPressureAngCoefC1))
//#define C2DIVBYC1 ((float)(gPressureLinCoefC2/gPressureAngCoefC1))
#define C1DIVBYLENGTH ((float)(gPressureAngCoefC1/PRESSUREMOVINGAVERAGELENGTH))



//#define SUMTOCMH2O(sumVar) ((sumVar)*C1DIVBYLENGTH-C2) //not used anymore



// Debug Macros
#define DEBUGISRTIMER 0

// To use libraries with hardware UART and Serial API
//#define txSerial Serial

static unsigned int gBufferPsensor[PRESSUREMOVINGAVERAGELENGTH]=
											{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static unsigned int gCumSum=0;

static bool gNewSampleAvailable = false;

//static float gPSensorCalibOffset=0; //not used anymore

//ADC value converted
static float gPSensorSampleWithCalib	= 0;


static float gPSensorPeep = 0;
static unsigned int gPSensorPeepInAdcValue=0;

//static unsigned long gTimePressureBelowPeep = 0;

static bool gCanStartCountingTimeFromPeepEvent = false;

// round and divide
//unsigned int movingAverage;//= ((gCumSum + (1<<(SHIFT_CTE_TO_DIV-1)))>>SHIFT_CTE_TO_DIV);

#if DEBUGISRTIMER
int tmpCnt = 0;
unsigned long tmpRef = 0;
unsigned long tmpCur;
#endif

#ifdef LCD_128x64
  char gTimer1PrintPsensor[LENGTH_GRAPH_PRESSURE_BUFFER];
  volatile int gTimer1BufferPrintIdx = 0;
#endif
/* Timer/Counter1 Compare Match A: it is used to control the pressure sensor
* sampling
* The OCR1A should be higher than OCR1B
*/
ISR(TIMER1_COMPA_vect) {
  //DebugISR
  /*if(statusInt)
  {
    txSerial.print(F("Já estava em uma int: "));
    txSerial.println(statusInt);
  }
  statusInt = 1;// */
  sei();
	static unsigned char bufferIdx = 0;
	float pSensorSampleRawValue;
#ifdef LCD_128x64
  static float graphPressureUpdateCount = FREQ_RESULT_PRESSURE_SENSOR;
#endif

#if DEBUG_CODE == 3
	unsigned long timeCur,timeRef=micros();
#endif

#if DEBUGISRTIMER
	++tmpCnt;
	if(tmpCnt==100){
		tmpCur = millis();
		txSerial.print(F("Time for 100 ISRs: "));
		txSerial.print(tmpCur-tmpRef);
		txSerial.println(F(" ms"));
	}
#endif

	// TODO: implement analogReference and analogRead with register operations?
 /*
	//analogReference(INTERNAL);
	// ADC read
	unsigned int sensorValue = analogRead(PIN_PRESSURE);
  sensorValue = analogRead(PIN_PRESSURE);
	//analogReference(DEFAULT); // */
	/// *
  //Keep the old values to restore context
  byte old = ADMUX;
  byte old2 = ADCSRA;
  ADMUX &= (0xF0); //Keep the voltage ref, but read from ADC0
  //7: ADC Enable, 6: ADC Start Conversion, 2:0 = 5:  ADC Prescaler to 32
  ADCSRA = 0xC5; // restart adc
  while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA |= (1 << 6); // request a new sample
  while(!(ADCSRA & 0x10)); // wait for adc to be ready
  byte m = ADCL; // fetch adc data
  byte j = ADCH;//
  //Restore the old status in case there was someone trying to obtain an sample
  ADMUX = old;
  ADCSRA = old2;
  unsigned int sensorValue = (j << 8) | m; // form into an int*/


	// CumSum to implement moving average
	// We sum the current value and subtract the oldest value
	gCumSum += sensorValue - gBufferPsensor[bufferIdx];

	// The new value is placed in the end of the circular buffer
	gBufferPsensor[bufferIdx] = sensorValue;

	if(bufferIdx<PRESSUREMOVINGAVERAGELENGTH-1){
		++bufferIdx;
	}else{
		bufferIdx=0;
	}

	// Division with rounding
	//movingAverage= ((gCumSum + (1<<(SHIFT_CTE_TO_DIV-1)))>>SHIFT_CTE_TO_DIV);

	//obtain the raw value
	pSensorSampleRawValue = ((float)gCumSum)/PRESSUREMOVINGAVERAGELENGTH;
	gPSensorSampleWithCalib = ADCVALUETOCMH2O(pSensorSampleRawValue, gPressureAngCoefC1, gPressureLinCoefC2);

#ifdef LCD_128x64

  //update the pressures of the graph according to the update rate
  graphPressureUpdateCount -= GRAPH_SENSOR_FREQ;
  if(graphPressureUpdateCount < 0)
  {
    graphPressureUpdateCount = FREQ_RESULT_PRESSURE_SENSOR;

    // The new value is placed in the end of the circular buffer to be able to print it
    if(gPSensorSampleWithCalib > MAX_GRAPH_PRESSURE_VALUE)
      gTimer1PrintPsensor[gTimer1BufferPrintIdx] = MAX_GRAPH_PRESSURE_VALUE - MIN_GRAPH_PRESSURE_VALUE; //40 - (-3) = 43
    else if(gPSensorSampleWithCalib < MIN_GRAPH_PRESSURE_VALUE)
      gTimer1PrintPsensor[gTimer1BufferPrintIdx] = 0; //equals MIN_GRAPH_PRESSURE_VALUE in the value
    else
    {
      	//Round the value.
    	gTimer1PrintPsensor[gTimer1BufferPrintIdx] = ROUND_CHAR(gPSensorSampleWithCalib);
    	gTimer1PrintPsensor[gTimer1BufferPrintIdx] -= MIN_GRAPH_PRESSURE_VALUE;

      //if(gPSensorSampleWithCalib >= 0)
      //  gTimer1PrintPsensor[gTimer1BufferPrintIdx] = (char)(gPSensorSampleWithCalib+0.5f) - MIN_GRAPH_PRESSURE_VALUE; //
      //else
      //  gTimer1PrintPsensor[gTimer1BufferPrintIdx] = (char)(gPSensorSampleWithCalib-0.5f) - MIN_GRAPH_PRESSURE_VALUE; //
    }

    if(gTimer1BufferPrintIdx>0){
      --gTimer1BufferPrintIdx;
    } else {
      gTimer1BufferPrintIdx=LENGTH_GRAPH_PRESSURE_BUFFER-1;
    }/*
    if(gTimer1BufferPrintIdx<LENGTH_GRAPH_PRESSURE_BUFFER-1){
      ++gTimer1BufferPrintIdx;
    } else {
      gTimer1BufferPrintIdx=0;
    }*/
  }
#endif
  /////


	if (gCanStartCountingTimeFromPeepEvent){
		gCanStartCountingTimeFromPeepEvent = false;
		//if(gPSensorSampleWithCalib<(gPSensorPeep-TRIGGER_START_COUNTING_TIME_CMH2O)){
		//	gTimePressureBelowPeep = millis();
		//}
	}

#if DEBUG_CODE == 2
	txSerial.println(gPSensorSampleWithCalib);
#endif

	gNewSampleAvailable = true;

	#if DEBUG_CODE == 3
	timeCur=micros();
	txSerial.println(timeCur-timeRef);
	#endif
  //statusInt = 0;
}
#ifdef LCD_128x64
int copyGraphBuffer(char *destBuffer)
{
  memcpy(destBuffer, gTimer1PrintPsensor, LENGTH_GRAPH_PRESSURE_BUFFER);
  return gTimer1BufferPrintIdx;
}
#endif


#endif //USE_PRESSURE_SENSOR

/* Timer/Counter1 Compare Match B: it is used to control the beep
* The OCR1B should be higher than OCR1B
*/
ISR(TIMER1_COMPB_vect) {
  sei();
	// TODO: implement beep update
	// writing 1 in PINB3 toogles the state of the pin.
	// See Section "14.2.2 Toggling the Pin" of the ATmega328 datasheet
	BUZZER_MCU_PORTPIN |= _BV(BUZZER_MCU_PIN);
	OCR1B += FREQ_OPTION_BUZZER;
	if(OCR1B>OCR1A){
		OCR1B = FREQ_OPTION_BUZZER;
	}
}

void timer1_init(){

#ifdef USE_PRESSURE_SENSOR
  // COM1A(1:0) = 00 // Normal port operation, OC1A disconnected
  // COM1B(1:0) = 00 // Normal port operation, OC1B disconnected
  // WGM1 (1:0) = 00 // WGM(3:0)=0100 Timer in CTC mode, TOP OCR1A
  TCCR1A = 0x00;

  // Frequency of the timer interrupt
  // OCR1A=210 and prescaler 1024 gives a frequency of aprox. 74 Hz in the CTC mode
  // OCR1A should be higher than OCR2B, in order to timer1 be applied to both
  // beep and pressure sensor
  OCR1A = FREQ_OPTION_PRESSURE_SENSOR;
#endif

  // ICNC1 = 0
  // ICES1 = 0
  // WGM1(3:2) = 01 // WGM(3:0)=0100 Timer in CTC mode, TOP OCR1A
  // CS1(2:0) = 101 // Prescaler = 1024
  TCCR1B = 0x0D;

  // In CTC mode with prescaler 1024, with OCR2A= 155 and OCR2B = 61,
  // the frequency of the beep will be approximately 126.01 Hz.
  // We could use OCR2B = 30 and have a frequency of approx. 252.02 Hz and so on.
  // you can check the possibilities here:
  //https://docs.google.com/spreadsheets/d/1eAW1s3_Mi-JtDXamgpoyo9Gr9R0HBD_HItr-XjfLl98/edit?usp=sharing
  // It is necessary that OCR2B be lower than  OCR2A.
  OCR1B = FREQ_OPTION_BUZZER;

  pinMode(BUZZER_ARDUINO_PIN, OUTPUT);
  digitalWrite(BUZZER_ARDUINO_PIN,LOW);
}

#ifdef USE_PRESSURE_SENSOR

void timer1_pSensor_sampling_start(){
	//at least one analog read (to configure registers) before reading from the timers
	analogRead(PIN_PRESSURE);

	// The Timer/Counter2 Compare Match B interrupt is enabled
	TIMSK1 |= 0x02;
#if DEBUGISRTIMER
	tmpRef = millis();
#endif
}

void timer1_pSensor_sampling_stop(){
	// The Timer/Counter2 Compare Match B interrupt is disabled
	TIMSK1 &= ~0x02;
}

//Pressure value converted to cmH2O using the C1 and C2 coeficients
float timer1_pSensor_read_cmH2O(){
	gNewSampleAvailable = false;
	//gPSensorSampleWithoutCalib = SUMTOCMH2O(gCumSum);
	//gPSensorSampleWithCalib = gPSensorSampleWithoutCalib;
	return gPSensorSampleWithCalib;
}

// This function is blocking
float timer1_pSensor_raw_start_value(int Nsamples){
	int cntSamples = 0;
	float sum = 0;
	float average;

	unsigned int sample;
	timer1_pSensor_sampling_stop();

	// Use internal ADC reference (1.1V)
	//analogReference(INTERNAL);

	for(cntSamples=0;cntSamples<Nsamples;++cntSamples){
		watchdog_reset();
		sample=analogRead(PIN_PRESSURE);
		sum+= sample;
		delay(10); //give a delay of aprox. 10 ms (sampling frequency ~=100 Hz)
		//txSerial.println(ADCVALUETOCMH2O(sample));
	}

	average = ((float)sum)/Nsamples;

	return average;
}
void set_calib_value(float pressureAngCoefC1, float pressureLinCoefC2)
{
	gPressureAngCoefC1 = pressureAngCoefC1;
	gPressureLinCoefC2 = pressureLinCoefC2;
}

bool timer1_pSensor_newSampleAvailable(){
	return gNewSampleAvailable;
}


PSensorTrigger timer1_pSensor_trigger(float triggerFallBelowPeep_cmH2O){
	//static unsigned long timeref = 0;
	//unsigned long timecur = 0;
	//timecur = millis();
	//static unsigned long t0;
	//unsigned long t1;
	//static float p0;
	float p1;
	PSensorTrigger ret = TRIGGERNONE;

	p1 = timer1_pSensor_read_cmH2O();
	//t1 = millis();

	if(p1< (gPSensorPeep - triggerFallBelowPeep_cmH2O)){
    //TODO: Rework this TRIGGERINSPIRATION Logic.
		//if((t1-gTimePressureBelowPeep) <
		//TRIGGER_PRESSURE_FALL_TIME_INSPIRATION_MS){
			ret = TRIGGERINSPIRATION;
		//}else{// */
		//	ret = TRIGGERFAULT;
		//}
	}

	return ret;

}



// Should be called in the middle of the expiration period
// Get enough samples and estimate their variance,
// then assign the average of the last samples to the PEEP
// return PeepInfo:
// The last sample is assinged to the pointer *newSample , except for the case
// when PeepInfo is PEEPNOTREADY
PeepInfo timer1_pSensor_calcPeep(bool firstTimeOfThisCycle, float *newSample){

	const int NsamplesToCalculatePEEP=4;
	//const float divByNsamples = 1/NsamplesToCalculatePEEP;
	float currP;

	static float maxP=-INFINITY;
	static float minP=INFINITY;
	static short cntSamplesForPeep = 0;
	static unsigned long tref;
	static float cumSumP=0;
	unsigned long t1 	= millis();
	PeepInfo peepInfoRet = PEEPNOTREADY;

	if(firstTimeOfThisCycle){
		// reset variables
		maxP				=-INFINITY;
		minP				= INFINITY;
		cntSamplesForPeep	= 0;
		tref 				= 0;
		cumSumP				= 0;
	}

	// if get enough samples and they have a low variance,
	// then assign the average of the last samples to the PEEP and return true

	if((t1-tref)>PEEP_SAMPLINGTIME_MS){
		if(timer1_pSensor_newSampleAvailable()){
			currP	= timer1_pSensor_read_cmH2O();
			cntSamplesForPeep++;
			// update time reference for the next call
			tref 	= t1;

			// update maxP and minP
			if(currP>maxP){
				maxP = currP;
			}

			if(currP<minP){
				minP = currP;
			}

			cumSumP += currP;
			*newSample = currP;
			peepInfoRet = PEEPNEWSAMPLE;
		}
	}

	if(NsamplesToCalculatePEEP==cntSamplesForPeep){
		// perform a rough estimation of the variance of the PEEP
		if((maxP-minP)>PEEP_THRESHOLD_VOLATILE_CMH2O){
			peepInfoRet = PEEPVOLATILE;
		}else{
			gPSensorPeep = cumSumP/NsamplesToCalculatePEEP;
			gPSensorPeepInAdcValue = CMH2OTOADCVALUE(gPSensorPeep, gPressureAngCoefC1, gPressureLinCoefC2);
			peepInfoRet = PEEPVALID;
		}
	}

	return peepInfoRet;
}

float timer1_pSensor_getPeep(){
	return gPSensorPeep;
}

void timer1_pSensor_enableTimeCounting(){
	gCanStartCountingTimeFromPeepEvent = true;
}

#endif //USE_PRESSURE_SENSOR

void timer1_beep_start(){
	TIMSK1 |= 0x04;
	gBeepIsRunnig = true;
}

void timer1_beep_stop(){
	TIMSK1 &= ~0x04;
	// Clear BUZZER PIN
	digitalWrite(BUZZER_ARDUINO_PIN,LOW);
	gBeepIsRunnig = false;
}

void timer1_beep_update(){
	const unsigned long beepTimeMs = 100;
	static bool beepOn = true;
	static unsigned long lastTime = 0;
	unsigned long currTime = millis();
	if(gBeepIsRunnig){
		currTime = millis();
		if((currTime - lastTime) > beepTimeMs){
			lastTime = currTime;
			if(beepOn){
				TIMSK1 |= 0x04; // enable interrupt
				beepOn = false;
			}else{
				TIMSK1 &= ~0x04; // disable interrupt
				digitalWrite(BUZZER_ARDUINO_PIN,LOW);
				beepOn = true;
			}
		}
	}
}

bool timer1_beepIsRunning(){
	return gBeepIsRunnig;
}


