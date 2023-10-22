#include "Arduino.h"
#include "alarm.h"
#include "global.h"
#include "pwm.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

#ifdef MOTOR_PWM

// COM2A(1:0) = 00 // Normal port operation, OC2A disconnected
// COM2B(1:0) = 01 // Toggle OC2B on Compare Match
// WGM2 (1:0) = 10 // CTC mode: WGM2 (2:0) =010
#define ENABLEPWMOUTPUT() {TCCR2A = 0x12;}

// clear COM1B(1:0)
#define DISABLEPWMOUTPUT() {TCCR2A &= 0xCF;}

// disable PWM output
//disable all interrupts associated with timer1
//reset timer
#define PWMSTOP() do{DISABLEPWMOUTPUT();\
TIMSK2=0;\
TCNT2 = 0;\
OCR2B = MAX_PWM_VALUE;\
OCR2A = OCR2B;\
}while(0)

// there are two ISRs per pulse.
static volatile bool g_firstpulse = true;
static volatile long *g_pulsePosition;
static volatile char *g_pulseDirection;
static volatile long *g_finalPosition;

#ifdef ACCELERATE_EVERY_PULSE
  static volatile int g_accelerateUntil; //positive or negative
  static volatile int g_startdeceleration; //positive or negative
  static volatile unsigned char g_targetFreqSpeed; //always positive
  static volatile unsigned char g_startFreqSpeed; //always positive
#endif
//int statusInt = 0;

/* Timer/Counter2 Compare Match B */
ISR(TIMER2_COMPB_vect) {
	#if DEBUG_CODE == 4
	unsigned int t1,tref = micros();
	#endif

  if(g_firstpulse){
    *g_pulsePosition += *g_pulseDirection;
    if(*g_pulsePosition==*g_finalPosition){
      PWMSTOP();
      // enable interrupt for the next interrupt
      g_firstpulse = true;
    }else{
      g_firstpulse = false;
    }
    #ifdef ACCELERATE_EVERY_PULSE
      //we must verify in what stage are we
      //are we in the acceleration period?
      //-1600 * 1 < g_accelerateUntil * 1 (-1500)
      //-1500 * 1 < g_startdeceleration * 1 (-1000)
      //-10 * -1 < g_accelerateUntil * -1 (-90)
      //-1000 * -1 < g_startdeceleration * -1 (-1500)

      // Do NOT try to optimize the line below.
      if(*g_pulsePosition * (*g_pulseDirection) < g_accelerateUntil *  (*g_pulseDirection))
      {
        if(OCR2B > g_targetFreqSpeed)//
        {
          	OCR2B = OCR2B - 1; //avoid function call in a ISR
		    OCR2A = OCR2B; // OCR2A should be also updated because it control timer reset
			#if DEBUG_CODE == 6
			  txSerial.println(OCR2A);
			#endif
        }
      } //did we reach full speed phase?
      // Do NOT try to optimize the line below.
      else if(*g_pulsePosition * (*g_pulseDirection) < g_startdeceleration * (*g_pulseDirection))
      {
        if(OCR2B != g_targetFreqSpeed)
        {
          OCR2B = g_targetFreqSpeed; //avoid function call in a ISR
          OCR2A = OCR2B; // every time OCR2B is updated, OCR2A has to be changed to the same value
		#if DEBUG_CODE == 6
		  txSerial.println(OCR2A);
		#endif
        }
      } else //we are in the deacceleration phase
      {
        if(OCR2B < g_startFreqSpeed)
        {
          OCR2B = OCR2B + 1; //avoid function call in a ISR
          OCR2A = OCR2B;
		#if DEBUG_CODE == 6
		  txSerial.println(OCR2A);
		#endif
        }
      }
    #endif
  }else{
    g_firstpulse = true;
  }

#if DEBUG_CODE == 4
	t1 = micros();
	txSerial.println(t1-tref);
#endif

  //statusInt = 0;
}
void pwm_configure_prescaler(unsigned char prescaler)
{
  TCCR2B = prescaler;
}
void pwm_init(volatile long *pulsePosition, volatile long *finalPosition, volatile char *pulseDirection){

  //obtain the pointers to the pulse, direction and final destionation
  g_pulsePosition = pulsePosition;
  g_finalPosition = finalPosition;
  g_pulseDirection = pulseDirection;

  // COM2A(1:0) = 00 // Normal port operation, OC2A disconnected
  // COM2B(1:0) = 01 // Toggle OC2B on Compare Match
  // WGM2 (1:0) = 10 // CTC mode: WGM2 (2:0) =010
  ENABLEPWMOUTPUT();

  // FOC2A = 0
  // FOC2B = 0
  // WGM2 (2) = 0
  // CS2 (2:0) = 101 // clk/128
  //TCCR2B = 0x05;
  pwm_configure_prescaler(PWM_PRESCALE_128);

  //Give the maximum TOP value for PWM1A
  OCR2B = MAX_PWM_VALUE;
  OCR2A = OCR2B;
}



void pwm_stop(){
	PWMSTOP();
}

#ifndef ACCELERATE_EVERY_PULSE
  void pwm_changeFreq(unsigned char freq){
    //  freq changes the number of rotation/ minute as shown here:
    https://docs.google.com/spreadsheets/d/1eAW1s3_Mi-JtDXamgpoyo9Gr9R0HBD_HItr-XjfLl98/edit?usp=sharing

    // Changes max. value of the TIMER/COunter2 (OCR2B)
      OCR2B = freq;
	  OCR2A = OCR2B;
  }
  void pwm_startRun()
  {
   // reset timer
    TCNT2 = 0;

    // set COM2A(1:0) to zero and matains all other bits in the same cfg
    ENABLEPWMOUTPUT();

    // OCIE2B: Timer/Counter2 Output Compare Match B Interrupt Enable
    TIMSK2  = 0x04;

    //g_isrunning = true;
  }
#else
  void pwm_startRun(unsigned char startFreqSpeed, unsigned char targetFreqSpeed)
  {
    g_startFreqSpeed = startFreqSpeed;
    g_targetFreqSpeed = targetFreqSpeed;
    int freqDifference = g_startFreqSpeed - g_targetFreqSpeed; //always positive, since the start freq is always greater than the end freq
    //long posDifference = *g_pulseDirection * (*g_finalPosition - *g_pulsePosition); //always positive

    //both are always positive
    // -(1) * (1*-1600 + 100) = 1500, -(1) * (1*-34 - 100) = 134,
    // -(-1) * (-1*-34 + 100) = 134, -(-1) * (-1*-1600 - 100) = 1500,

    // -(-1) * (-1*1600 + 100) = 1500, -(1) * (1*-34 - 100) = 134,
    // -(1) * (-1*-34 + 100) = 134, -(-1) * (-1*-1600 - 100) = 1500,

    g_accelerateUntil = ((*g_pulsePosition) + (*g_pulseDirection)*freqDifference);
    g_startdeceleration = ((*g_finalPosition) - (*g_pulseDirection)*freqDifference);

    /*
    #if DEBUG_CODE != 0
      txSerial.print(F("freqDifference: "));
      txSerial.print(freqDifference);
      txSerial.print(F(", g_startFreqSpeed: "));
      txSerial.print(g_startFreqSpeed);
      txSerial.print(F(", g_targetFreqSpeed: "));
      txSerial.print(g_targetFreqSpeed);
      txSerial.print(F(", g_accelerateUntil: "));
      txSerial.print(g_accelerateUntil);
      txSerial.print(F(", g_startdeceleration: "));
      txSerial.println(g_startdeceleration);
    #endif// */


    // reset timer
    TCNT2 = 0;

    OCR2B = g_startFreqSpeed;
	OCR2A = OCR2B;
    //g_isrunning = true;

    // set COM2A(1:0) to zero and matains all other bits in the same cfg
    ENABLEPWMOUTPUT();

    // OCIE2B: Timer/Counter2 Output Compare Match B Interrupt Enable
    TIMSK2  = 0x04;

  }
#endif

#endif
