#include "motor.h"
#include "pwm.h"
#include "configmanager.h"


void calculateMotorParam()
{
  #ifdef DEBUG_CODE
    Serial.println("Calculating the inspiration and expiration position, plus the motor speed.");
  #endif

  //calculate the steps per unit of configuration (1-1 to 15-1)
  //note that it MIGHT BE equal to the end limit swicth
  double stepsPerUnit = ((gStartConfig.endLimitSwitchPos)/(double)14);
  //calculate the positions of inspiration and expiration

  gMotorParameters.stepPosExp = stepsPerUnit * (gActiveRespCfg.pos_exp-1);
  
  gMotorParameters.stepPosIns = gMotorParameters.stepPosExp - (gMotorParameters.stepPosExp * 0.01f * gActiveRespCfg.volume);
  gMotorParameters.stepPosIns += gStartConfig.endLimitSwitchPos/FRAC_SWITCH_LIMIT_OFFSET; //Note that we do not need to multiply by ROTATIONAL_DIRECTION, since endLimitSwitchPos inverts for insp/exp already
  gMotorParameters.stepPosExp -= gStartConfig.endLimitSwitchPos/FRAC_SWITCH_LIMIT_OFFSET;

  //first we calculate the number of steps for inspiration and expiration
  long breathStepsPerInsOrExp = ROTATIONAL_DIRECTION*(gMotorParameters.stepPosExp - gMotorParameters.stepPosIns);

  //calculate the stepsPerSecond according to bpm (ventilation per minute)
  //stepsPerSecond = (steps per minute)/60
  float stepsPerSecondPerInsOrExp = (gActiveRespCfg.bpm * breathStepsPerInsOrExp)/(float)60;


  //Note that both insp. and expiration will have the same amount of steps
  //inspiration must perform those steps FASTER than expiration, in order to
  //do it at a faster ratio

  //Now, we have the ratio between inpiration and expiration
  //If 1:1, 50% of the TIME will be for each side
  //If 1:2, 1/3 will be insp. and 2/3 will be exp, thus:

  //The inspiration must do breathStepsPerInsOrExp AVERAGE per second,
  //But it will have only 1/(1+ratio) of the total time do do it
  //thus, it must multiply stepsPerSecondPerInsOrExp by (1+ratio)
  gMotorParameters.insVel = stepsPerSecondPerInsOrExp * (1 + gActiveRespCfg.ie_ratio);

  //The expiration must do breathStepsPerInsOrExp AVERAGE per second,
  //But it will have only ratio/(1+ratio) of the total time do do it
  //thus, it must multiply stepsPerSecondPerInsOrExp by (1+ratio)/ratio.
  gMotorParameters.expVel = stepsPerSecondPerInsOrExp * ((1 + gActiveRespCfg.ie_ratio)/gActiveRespCfg.ie_ratio);

  float timePerInsAndExp = 60/(float)gActiveRespCfg.bpm;
  float insTime = timePerInsAndExp/(1 + gActiveRespCfg.ie_ratio);
  float expTime = insTime *  gActiveRespCfg.ie_ratio;


  #if MOTOR_ACCELERATION > 0
    #ifndef MOTOR_PWM
      //The motor must accelerate in 1/x of the time
      //the final speed must be adjusted in order to compensate the time lost in the acceleration/deacceleration proccess.
      gMotorParameters.insVel *= 1.02f * MOTOR_ACCELERATION/(double)(MOTOR_ACCELERATION-1);
      gMotorParameters.expVel *= 1.02f * MOTOR_ACCELERATION/(double)(MOTOR_ACCELERATION-1);

      //if there a fixed expiration phase speed?
      #if FIXED_EXPIRATION_SPEED > 0
        //the FIXED_EXPIRATION_SPEED is higher than the calculated one?
        if(FIXED_EXPIRATION_SPEED > gMotorParameters.expVel)
          gMotorParameters.expVel = FIXED_EXPIRATION_SPEED;
      #endif

      //the acceleration must be set so that it reachs the full speed in time/x
      gMotorParameters.insAcel = gMotorParameters.insVel/(insTime/MOTOR_ACCELERATION);
      gMotorParameters.expAcel = gMotorParameters.expVel/(expTime/MOTOR_ACCELERATION);
    #endif
  #endif

  #ifdef MOTOR_PWM
    //if there a fixed expiration phase speed?
    #if FIXED_EXPIRATION_SPEED > 0
      //the FIXED_EXPIRATION_SPEED is higher than the calculated one?
      if(FIXED_EXPIRATION_SPEED > gMotorParameters.expVel)
        gMotorParameters.expVel = FIXED_EXPIRATION_SPEED;
    #endif
///*
    #ifdef DEBUG_CODE
      Serial.print("Without accel: gMotorParameters.insVel: ");
      Serial.print(gMotorParameters.insVel);
      Serial.print("gMotorParameters.expVel: ");
      Serial.println(gMotorParameters.expVel);
    #endif//*/
    
    #if MOTOR_ACCELERATION > 0
      //first we must find the frequency for the speed without acceleration
      //note that this calculation will be done again after we find the final desired speed
      gMotorParameters.insTargetSpeedFreq = findFreqSpeed(gMotorParameters.insVel, &gMotorParameters.insPWMPrescaler);
      gMotorParameters.expTargetSpeedFreq = findFreqSpeed(gMotorParameters.expVel, &gMotorParameters.expPWMPrescaler);

      //in order to obtain more accurate values, we save the start freq speed
      if(((int)gMotorParameters.insTargetSpeedFreq)+MAX_RANGE_PWM_VALUE < MAX_PWM_VALUE)
        gMotorParameters.insStartSpeedFreq = gMotorParameters.insTargetSpeedFreq+MAX_RANGE_PWM_VALUE;
      else
        gMotorParameters.insStartSpeedFreq = MAX_RANGE_PWM_VALUE;
        
      if(((int)gMotorParameters.expTargetSpeedFreq)+MAX_RANGE_PWM_VALUE < MAX_PWM_VALUE)
        gMotorParameters.expStartSpeedFreq = gMotorParameters.expTargetSpeedFreq+MAX_RANGE_PWM_VALUE;
      else
        gMotorParameters.expStartSpeedFreq = MAX_RANGE_PWM_VALUE;

      #ifndef ACCELERATE_EVERY_PULSE

        //calculate the mean velocity for the acceleration stage
        //If there is an acceleration, we must now find the speed for the start acceleration
        float meanInsAccelSpeed = calcMeanSpeed(gMotorParameters.insStartSpeedFreq-1, gMotorParameters.insTargetSpeedFreq-1, gMotorParameters.insPWMPrescaler);///2;
        float meanExpAccelSpeed = calcMeanSpeed(gMotorParameters.expStartSpeedFreq-1, gMotorParameters.expTargetSpeedFreq-1, gMotorParameters.expPWMPrescaler);///2;

        //we can now calculate the desired final speed to compensate acceleration
        //the final speed must be adjusted in order to compensate the time lost in the acceleration/deacceleration proccess.
        gMotorParameters.insVel = ((gMotorParameters.insVel * MOTOR_ACCELERATION)-(2*meanInsAccelSpeed))/(double)(MOTOR_ACCELERATION-2);
        gMotorParameters.expVel = ((gMotorParameters.expVel * MOTOR_ACCELERATION)-(2*meanExpAccelSpeed))/(double)(MOTOR_ACCELERATION-2);
      #else
        float meanAccelSpeed, accelTime;
        calcMeanSpeed(&meanAccelSpeed, &accelTime, &gMotorParameters.insStartSpeedFreq, &gMotorParameters.insTargetSpeedFreq, insTime, gMotorParameters.insPWMPrescaler);
        if(accelTime != 0)
          gMotorParameters.insVel = ((2*meanAccelSpeed*accelTime) - (insTime*gMotorParameters.insVel))/((2*accelTime)-insTime);

        calcMeanSpeed(&meanAccelSpeed, &accelTime, &gMotorParameters.expStartSpeedFreq, &gMotorParameters.expTargetSpeedFreq, expTime, gMotorParameters.expPWMPrescaler);
        if(accelTime != 0)
          gMotorParameters.expVel = ((2*meanAccelSpeed*accelTime) - (expTime*gMotorParameters.expVel))/((2*accelTime)-expTime);
      #endif
    #endif


    gMotorParameters.insTargetSpeedFreq = findFreqSpeed(gMotorParameters.insVel, &gMotorParameters.insPWMPrescaler);
    gMotorParameters.expTargetSpeedFreq = findFreqSpeed(gMotorParameters.expVel, &gMotorParameters.expPWMPrescaler);

    //microTimes are only used when we are not accelerating every pulse
    #ifndef ACCELERATE_EVERY_PULSE
      gMotorParameters.insMicroTime = insTime  * 1000000; //*1000000 to convert to microsec
      gMotorParameters.expMicroTime = expTime  * 1000000; //*1000000 to convert to microsec
      #if MOTOR_ACCELERATION > 0
        gMotorParameters.insMicroAcelTime = (insTime/MOTOR_ACCELERATION) * 1000000;
        gMotorParameters.expMicroAcelTime = (expTime/MOTOR_ACCELERATION) * 1000000;
      #else
        gMotorParameters.insMicroAcelTime = 0;
        gMotorParameters.expMicroAcelTime = 0;
      #endif
    #endif

    #ifdef DEBUG_CODE
      Serial.print("gMotorParameters.insVel: ");
      Serial.print(gMotorParameters.insVel);
      Serial.print(", insTargetSpeedFreq: ");
      Serial.println(gMotorParameters.insTargetSpeedFreq);
      Serial.print("gMotorParameters.expVel: ");
      Serial.print(gMotorParameters.expVel);
      Serial.print(", expTargetSpeedFreq: ");
      Serial.println(gMotorParameters.expTargetSpeedFreq);
    #endif
  #endif


  #ifdef DEBUG_CODE
    //Serial.print("insTime: ");
    //Serial.println(insTime);
/*
    Serial.print("breathStepsPerInsOrExp: ");
    Serial.println(breathStepsPerInsOrExp);*/
    #ifndef MOTOR_PWM
      Serial.print("gMotorParameters.insAcel: ");
      Serial.println(gMotorParameters.insAcel);
    #endif/*
    Serial.print("stepsPerSecond: ");
    Serial.println(stepsPerSecondPerInsOrExp);*/
    Serial.print("Inspiration pos:");
    Serial.print(gMotorParameters.stepPosIns);
    Serial.print(", vel:");
    Serial.print(gMotorParameters.insVel);
    Serial.print("Expiration pos:");
    Serial.print(gMotorParameters.stepPosExp);
    Serial.print(", vel:");
    Serial.println(gMotorParameters.expVel);
  #endif
}
State funcS2(State oldState)
{
  CmdIhm checkVal = NONE;

  // Wait Confirmation form IHM
  while(checkVal != SELECT)
  {
    checkVal = ihm_check();
    //is there any changes in the active configuration?
    if(checkVal == NEWDATA)
      saveRespConfiguration(&gActiveRespCfg, gStartConfig.activeConfigPos);
  }

  //calculate the respiration parameters
  calculateMotorParam();

  return S3;
}
