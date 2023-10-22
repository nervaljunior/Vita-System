#include "global.h"
#include "configmanager.h"
#include "motor.h"

void configureExpMotor()
{/*
  #ifdef DEBUG_CODE
    Serial.print("Current Pos: ");
    Serial.print(motorGetPosition());
    Serial.print(", Target Pos: ");
    Serial.println(gMotorParameters.stepPosExp);
  #endif*/
  
#ifndef MOTOR_PWM
  motorMoveTo(gMotorParameters.stepPosExp, EXPIRATION_SIGN, gMotorParameters.expVel, gMotorParameters.expAcel);
#else
  #ifndef ACCELERATE_EVERY_PULSE
    motorPWMMoveTo(gMotorParameters.stepPosExp, EXPIRATION_SIGN, gMotorParameters.expTargetSpeedFreq, gMotorParameters.expStartSpeedFreq, gMotorParameters.expMicroTime, gMotorParameters.expMicroAcelTime, gMotorParameters.expPWMPrescaler);
  #else
    motorPWMMoveTo(gMotorParameters.stepPosExp, EXPIRATION_SIGN, gMotorParameters.expTargetSpeedFreq, gMotorParameters.expStartSpeedFreq, gMotorParameters.expPWMPrescaler);
  #endif
#endif
}


State verifyStartBreath(State oldState)
{
  State state;
  unsigned long now;
  //compute the differences at the start of the next breath
  if(gInitialBreathTime > 0)
  {
    //we should now be sure that we are not too fast
    while(1)
    {
      now = millis();
      //-1 millis to allow the software to reach the inspiration phase again in time
      if((now - gInitialBreathTime) > (1000*(60/(float)gActiveRespCfg.bpm))-1)
        break;
        
      //Verify overflow
      if(now < gInitialBreathTime) 
        break;
      
      state = verifyIHM(oldState, EXPIRATION_SIGN);
      if(state != NA)
        return state;
        
    }    
  }
  return NA;
}
State funcS4(State oldState)
{
  State state = NA;
  configureExpMotor();

  //note that the gMotorParameters.stepPosIns can be changed inside the while
  while(motorDistanceToGo() != 0)
  {
    //verify if we reached the end limit switch
    if(digitalRead(LIMIT_SWITCH_END_PIN) == 0)
    {
        #ifdef DEBUG_CODE
          Serial.println("End Limit Switch reached.");
        #endif
        //for whatever the reason, we did reach the end limit switch
        //set the current position that correponds to the end limit switch
        motorSetPosition(gStartConfig.endLimitSwitchPos);
        motorStop();
        //exit the while
        break;
    }
    motorRun();
    
    state = verifyIHM(oldState, EXPIRATION_SIGN);
    if(state != NA)
      return state;
  }
  motorStop();
  motorUpdatePosition();

  state = verifyStartBreath(oldState);
  if(state != NA)
    return state;
  
  return S3;
}
