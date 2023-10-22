#include "global.h"
#include "configmanager.h"
#include "motor.h"



State verifyIHM(State oldState, InsExpSign phase)
{
  CmdIhm checkVal;
  // Wait Confirmation form IHM
  checkVal = ihm_check();
  //TODO: what to do if SELECT is pressed? Also, checkVal could have other values like STOP

  //is there any changes in the active configuration?
  if(checkVal == NEWDATA)
  {
    //save the configuration in the EEPROM
    saveRespConfiguration(&gActiveRespCfg, gStartConfig.activeConfigPos);
    //calculate the respiration parameters
    calculateMotorParam();
    if(phase == INSPIRATION_SIGN)
      configureInspMotor();
    else
      configureExpMotor();
  } else if(checkVal == SELECT && (oldState == S4 || oldState == S3))
  {
        motorStop();
        motorUpdatePosition();
        delay(1000);
        return S2;
  }
  return NA;
}
void configureInspMotor()
{/*
  #ifdef DEBUG_CODE
    Serial.print("Current Pos: ");
    Serial.print(motorGetPosition());
    Serial.print(", Target Pos: ");
    Serial.println(gMotorParameters.stepPosIns);
  #endif*/

#ifndef MOTOR_PWM
  motorMoveTo(gMotorParameters.stepPosIns, INSPIRATION_SIGN, gMotorParameters.insVel, gMotorParameters.insAcel);
#else
  #ifndef ACCELERATE_EVERY_PULSE
    motorPWMMoveTo(gMotorParameters.stepPosIns, INSPIRATION_SIGN, gMotorParameters.insTargetSpeedFreq, gMotorParameters.insStartSpeedFreq, gMotorParameters.insMicroTime, gMotorParameters.insMicroAcelTime, gMotorParameters.insPWMPrescaler);
  #else
    motorPWMMoveTo(gMotorParameters.stepPosIns, INSPIRATION_SIGN, gMotorParameters.insTargetSpeedFreq, gMotorParameters.insStartSpeedFreq, gMotorParameters.insPWMPrescaler);    
  #endif
#endif
}

State funcS3(State oldState)
{
  unsigned long now;
  State state = NA;
  configureInspMotor();

  while(motorDistanceToGo() != 0)
  {
    //if there is a start limit switch
    #ifdef START_LIMIT_SWITCH
      //verify if we reached it
      if(digitalRead(LIMIT_SWITCH_INI_PIN) == 0)
      {
        #ifdef DEBUG_CODE
          Serial.println("Start Limit Switch reached.");
        #endif
          //for whatever the reason, we did touch the start limit switch
          //set the current position that correponds to the start limit switch
          motorSetPosition(0);
          motorStop();
          //exit the while
          break;
      }
    #endif

    motorRun();
    state = verifyIHM(oldState, INSPIRATION_SIGN);
    if(state != NA)
      return state;
  }  
  motorStop();
  motorUpdatePosition();
  
  return S4;
}
