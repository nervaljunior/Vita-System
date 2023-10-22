#include "Arduino.h"
#include "global.h"
#include "pwm.h"

#ifdef MOTOR_PWM

// COM2A(1:0) = 01 // Toggle OC2A on Compare Match
// COM2B(1:0) = 00 // Normal port operation, OC2B disconnected
// WGM2 (1:0) = 10
#define ENABLEPWMOUTPUT() {TCCR2A = 0x42;}
#define DISABLEPWMOUTPUT() {TCCR2A &= 0x3F;}

// there are two ISRs per pulse.
static volatile bool g_firstpulse = true;
static volatile int *g_pulsePosition;
static volatile char *g_pulseDirection;
static volatile int *g_finalPosition;

#ifdef ACCELERATE_EVERY_PULSE
  static volatile int g_accelerateUntil; //positive or negative
  static volatile int g_startdeceleration; //positive or negative
  static volatile unsigned char g_targetFreqSpeed; //always positive
  static volatile unsigned char g_startFreqSpeed; //always positive
#endif

/* Timer/Counter2 Compare Match A */
ISR(TIMER2_COMPA_vect) {
  if(g_firstpulse){
    *g_pulsePosition += *g_pulseDirection;
    #ifdef ACCELERATE_EVERY_PULSE
      //we must verify in what stage are we
      //are we in the acceleration period?      
      //-1600 * 1 < g_accelerateUntil * 1 (-1500)
      //-1500 * 1 < g_startdeceleration * 1 (-1000)
      //-10 * -1 < g_accelerateUntil * -1 (-90)
      //-1000 * -1 < g_startdeceleration * -1 (-1500)
            
      if(*g_pulsePosition * (*g_pulseDirection) < g_accelerateUntil *  (*g_pulseDirection))
      {
        if(OCR2A > g_targetFreqSpeed)// 
        {
          OCR2A = OCR2A - 1; //avoid function call in a ISR
          #ifdef DEBUG_CODE_PULSES
          Serial.print("acc: ");
          Serial.println(OCR2A);
          #endif
        }
      } //did we reach full speed phase?
      else if(*g_pulsePosition * (*g_pulseDirection) < g_startdeceleration *  (*g_pulseDirection))
      {
        if(OCR2A != g_targetFreqSpeed)
        {
          OCR2A = g_targetFreqSpeed; //avoid function call in a ISR
          #ifdef DEBUG_CODE_PULSES
          Serial.print("fixsp: ");
          Serial.println(OCR2A);
          #endif
        }
      } else //we are in the deacceleration phase
      {
        if(OCR2A < g_startFreqSpeed)
        {
          OCR2A = OCR2A + 1; //avoid function call in a ISR
          #ifdef DEBUG_CODE_PULSES
          Serial.print("dec: ");
          Serial.println(OCR2A);
          #endif
        }
      }
    #endif      
    if(*g_pulsePosition==*g_finalPosition){
      pwm_stop();
      // enable interrupt for the next interrupt
      g_firstpulse = true;
    }else{
      g_firstpulse = false;
    }
  }else{
    g_firstpulse = true;
  }
}
void pwm_configure_prescaler(unsigned char prescaler)
{
  TCCR2B = prescaler;//0x05;  
}
void pwm_init(int *pulsePosition, int *finalPosition, char *pulseDirection){

  //obtain the pointers to the pulse, direction and final destionation
  g_pulsePosition = pulsePosition;
  g_finalPosition = finalPosition;
  g_pulseDirection = pulseDirection;

  // configure registers
  
  // We are going to use OCR1B
  
  // configure ISR when OCR2 os achieved timer achieves TOP
  // Mode 2
  // Timer/Counter Mode of Operation: CTC
  // TOP: OCRA
  // Update of OCRx at: Immediate
  // TOV Flag Set on MAX
  // WGM2 (2:0) = 010
  // Note that WGM2 is configured in TCCR2A and TCCR2B
  
  
  // COM2A(1:0) = 01 // Clear OC2A on Compare Match (Set output tp low level)
  // COM2B(1:0) = 00 // Normal port operation, OC2B disconnected
  // WGM2 (1:0) = 11
  //TCCR2A = 0x43;
  ENABLEPWMOUTPUT();

   
  // FOC2A = 0 
  // FOC2B = 0
  // WGM2 (2) = 0
  // CS2 (2:0) = 101 // clk/128
  //TCCR2B = 0x05;
  pwm_configure_prescaler(PWM_PRESCALE_128);
    
  // ASSR is not configured. All default values are 0
  
  //Give the maximum TOP value for PWM2A
  OCR2A = 255;
}



void pwm_stop(){

  // disable timer
  // set COM2A(1:0) to zero and matains all other bits in the same cfg
  //TCCR2A &= 0x3F;
  DISABLEPWMOUTPUT();

  //disable all interrupts associated with timer2
  TIMSK2 = 0;
  //reset timer
  TCNT2 = 0;
  OCR2A = 255;
  //g_isrunning=false;
}

#ifndef ACCELERATE_EVERY_PULSE
  void pwm_changeFreq(unsigned char freq){
    //  freq changes the number of rotation/ minute as shown here:
    https://docs.google.com/spreadsheets/d/1eAW1s3_Mi-JtDXamgpoyo9Gr9R0HBD_HItr-XjfLl98/edit?usp=sharing
    
    // Changes max. value of the TIMER/COunter2 (OCR2A)
      OCR2A = freq;
  }
  void pwm_startRun()
  {
   // reset timer
    TCNT2 = 0;
    
    // set COM2A(1:0) to zero and matains all other bits in the same cfg
    ENABLEPWMOUTPUT();
  
    // Enable Interrupt
    TIMSK2  = 0x02;
    
    //g_isrunning = true;
  }
#else
  void pwm_startRun(unsigned char startFreqSpeed, unsigned char targetFreqSpeed)
  {
    g_startFreqSpeed = startFreqSpeed;
    g_targetFreqSpeed = targetFreqSpeed;
    long freqDifference = g_startFreqSpeed - g_targetFreqSpeed; //always positive, since the start freq is always greater than the end freq
    //long posDifference = *g_pulseDirection * (*g_finalPosition - *g_pulsePosition); //always positive

    //both are always positive
    // -(1) * (1*-1600 + 100) = 1500, -(1) * (1*-34 - 100) = 134,
    // -(-1) * (-1*-34 + 100) = 134, -(-1) * (-1*-1600 - 100) = 1500,
    
    // -(-1) * (-1*1600 + 100) = 1500, -(1) * (1*-34 - 100) = 134,
    // -(1) * (-1*-34 + 100) = 134, -(-1) * (-1*-1600 - 100) = 1500,
    
    g_accelerateUntil = ((*g_pulsePosition) + (*g_pulseDirection)*freqDifference);
    g_startdeceleration = ((*g_finalPosition) - (*g_pulseDirection)*freqDifference); 

    /*
    #ifdef DEBUG_CODE
      Serial.print("freqDifference: ");
      Serial.print(freqDifference);
      Serial.print(", g_startFreqSpeed: ");
      Serial.print(g_startFreqSpeed);
      Serial.print(", g_targetFreqSpeed: ");
      Serial.print(g_targetFreqSpeed);
      Serial.print(", g_accelerateUntil: ");
      Serial.print(g_accelerateUntil);
      Serial.print(", g_startdeceleration: ");
      Serial.println(g_startdeceleration);
    #endif//*/

    
    // reset timer
    TCNT2 = 0;

    OCR2A = g_startFreqSpeed;    
    //g_isrunning = true;
    
    // set COM2A(1:0) to zero and matains all other bits in the same cfg
    ENABLEPWMOUTPUT();
  
    // Enable Interrupt
    TIMSK2  = 0x02;
    
  }
#endif

#endif
