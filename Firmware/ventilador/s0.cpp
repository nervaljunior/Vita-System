#include "global.h"
#include "ihm.h"
#include "motor.h"




//Runs the motor clockwise until it finds the limit switch (0 = start, 1 = end switch)
bool findLimitSwitch(int endSwitch)
{
  int pin;
  InsExpSign sign;
  
  if(endSwitch == 0)
  {
    pin = LIMIT_SWITCH_INI_PIN;
    // sets the motor to run clockwise until it finds the initial limit switch
    // note that it is just the number of steps not the atual position
    // Note that anticlockwise  = closing = negative, but it also depend on ROTATIONAL_DIRECTION
    sign = INSPIRATION_SIGN;
  }
  else
  {
    pin = LIMIT_SWITCH_END_PIN;
    // sets the motor to run clockwise until it finds the end limit switch
    // note that it is just the number of steps not the atual position
    sign = EXPIRATION_SIGN;
  }

#ifndef MOTOR_PWM
  //note that the ROTATIONAL_DIRECTION*sign* at the position parameters is necessary
  motorMoveTo(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS, sign, CONFIG_MOTOR_SPEED, CONFIG_MOTOR_SPEED);
#else
  unsigned char pwmPrescaler;
  unsigned int targetFreqSpeed = findFreqSpeed(CONFIG_MOTOR_SPEED, &pwmPrescaler);
  unsigned long totalTime = 1000000 * (MAXIMUM_START_SEQ_NUM_STEPS/STEPS_PER_REVOLUTION); //in microseconds
  /*
  #ifdef DEBUG_CODE
    Serial.print("targetFreqSpeed: ");
    Serial.println(targetFreqSpeed);
    Serial.print("totalTime: ");
    Serial.println(totalTime);
  #endif*/
  #if MOTOR_ACCELERATION == 0
    motorPWMMoveTo(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE, totalTime, 0, pwmPrescaler);
  #else
    #ifndef ACCELERATE_EVERY_PULSE
      motorPWMMoveTo(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE,totalTime, totalTime/MOTOR_ACCELERATION, pwmPrescaler);
    #else
      motorPWMMoveTo(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE, pwmPrescaler);
    #endif
  #endif
#endif
  /*
  #ifndef MOTOR_PWM
    #if MOTOR_ACCELERATION == 0
      // Code used when there is no acceleration
      motor.move(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS);
      motor.setSpeed(ROTATIONAL_DIRECTION*sign*CONFIG_MOTOR_SPEED); //positive or negative
    #else
      // Code with acceleration
      motor.setAcceleration(CONFIG_MOTOR_SPEED); //always positive, fixed at motor speed for now
      motor.setMaxSpeed(CONFIG_MOTOR_SPEED); //always positive
      motor.move(ROTATIONAL_DIRECTION*sign*MAXIMUM_START_SEQ_NUM_STEPS);
    #endif
  #endif
  */

  int sTime = millis();
  long calls = 0;
  ihm_initScreen();
  while(motorDistanceToGo() != 0)  
  {

    // if we got to the limit switch
    if(digitalRead(pin) == 0)
    {
      motorStop();
      return true;
    }
      
    
    motorRun();
    calls++;
  }

  #ifdef DEBUG_CODE
    Serial.print("Couldn't find the switch in calls/sec:");
    float difTime = (millis()-sTime)/(float)1000;
    Serial.println(calls/difTime);
  #endif
    int startTime = millis();
  motorStop();
  return false;
}

// Adjust the equipment.
// Block all others functions.
State funcS0(State oldState)
{

  #ifdef DEBUG_CODE
    Serial.println("Initial Configuration Screen");
  #endif

  #ifdef START_LIMIT_SWITCH
    if(findLimitSwitch(0) == false)
    {
      //something went wrong. Stopping the proccess
      reportErr("Err: No INI Switch");
    }
    // zero out the current position
    motorSetPosition(0);
    
    if(findLimitSwitch(1) == false)
    {
      //something went wrong. Stopping the proccess
      reportErr("Err: No END Switch");
    }
    //might be positive or negative, dependending on the ROTATIONAL_DIRECTION define.
    gStartConfig.endLimitSwitchPos = motorGetPosition();
    motorSetPosition(gStartConfig.endLimitSwitchPos);

    #ifdef DEBUG_CODE
      Serial.print("Position of the end limit switch: ");
      Serial.println(gStartConfig.endLimitSwitchPos);
    #endif
  #else
    gStartConfig.endLimitSwitchPos = ROTATIONAL_DIRECTION*TOTAL_NUMBER_STEPS;

    if(findLimitSwitch(1) == false)
    {
      //something went wrong. Stopping the proccess
      reportErr("Cannot Find END Switch");
    }
  #endif
  motorStop();
  //finally, we set the current position to that which represents the end limit swicht position
  motorSetPosition(gStartConfig.endLimitSwitchPos);
	return S1;
}
