#ifndef MOTOR_H_
#define MOTOR_H_

#include "global.h"

void motorInit();
long motorDistanceToGo();
long motorRun();
void motorStop();
long motorGetPosition();

#ifndef MOTOR_PWM
  void motorMoveTo(long targetPosition, InsExpSign inspExp, float velocity, float acceleration);
  void motorUpdatePosition();
#else
  void motorUpdatePosition();
  int findFreqSpeed(double targetSpeed, unsigned char *pwmPrescaler);
  #ifndef ACCELERATE_EVERY_PULSE
    float calcMeanSpeed(unsigned char startFreq, unsigned char endFreq, unsigned char pwmPrescaler);
    void motorPWMMoveTo(long targetPosition, InsExpSign inspExp, unsigned char targetFreqSpeed, unsigned char startFreqSpeed, unsigned long motorTotalTime, unsigned long motorAccelerationTime, unsigned char pwmPrescaler);
  #else
    void calcMeanSpeed( float *meanAccelSpeed, float *stentTime,  unsigned char *startSpeedFreq,  unsigned char *targetSpeedFreq, float totalTime, unsigned char pwmPrescaler);
    void motorPWMMoveTo(long targetPosition, InsExpSign inspExp, unsigned char targetFreqSpeed, unsigned char startFreqSpeed, unsigned char pwmPrescaler);
  #endif
#endif
void motorSetPosition(long newPosition);


#endif
