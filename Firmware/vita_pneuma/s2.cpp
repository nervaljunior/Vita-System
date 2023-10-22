#include "alarm.h"
#include "motor.h"
#include "timer1.h"
#include "pwm.h"
#include "configmanager.h"
#include "ihm.h"
#include "pressure.h"
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

static void calculateCycleParam(CycleType cycleType)
{
	//calculate the positions of inspiration and expiration
	if(cycleType == NORMAL_CYCLE)
	{
		//Normal cycles must take into account the end limit switch for the expiration phase, minus the STEPS_EXPIRATION_POSITION_OFFSET
		//in order to do not reach the expiration limit switch
		gMotorParameters.cycles[cycleType].stepPosExp = gActiveRespCfg.endLimitSwitchPos;
		gMotorParameters.cycles[cycleType].stepPosExp -= ROTATIONAL_DIRECTION*STEPS_EXPIRATION_POSITION_OFFSET;
		gMotorParameters.cycles[cycleType].stepPosIns = gMotorParameters.cycles[cycleType].stepPosExp - (ROTATIONAL_DIRECTION*gActiveRespCfg.volume_pulses);
		gMotorParameters.cycles[cycleType].posExpVol = gMotorParameters.cycles[cycleType].stepPosExp; //same as expiration position, used for volume calculation
	} else {
		//Test cycles must take into account the end limit switch for the expiration phase, plus the STEPS_TEST_CYCLE_INCREASE steps.
		gMotorParameters.cycles[cycleType].stepPosExp = gActiveRespCfg.endLimitSwitchPos;
		gMotorParameters.cycles[cycleType].stepPosExp += ROTATIONAL_DIRECTION*STEPS_TEST_CYCLE_INCREASE;
		//The inspiration limit must be used so that it takes into account the fact that it started at the expiration end limit switch
		//Note that the volume_pulses must then be summed with STEPS_TEST_CYCLE_POSITION_OFFSET, since STEPS_TEST_CYCLE_POSITION_OFFSET is the start of the ambu bag.
		//and steps before that do not deliver any volume.
		gMotorParameters.cycles[cycleType].stepPosIns = gMotorParameters.cycles[cycleType].stepPosExp - (ROTATIONAL_DIRECTION*(gActiveRespCfg.volume_pulses+STEPS_TEST_CYCLE_POSITION_OFFSET));
		gMotorParameters.cycles[cycleType].posExpVol = gMotorParameters.cycles[cycleType].stepPosExp - STEPS_TEST_CYCLE_POSITION_OFFSET; //stepPosExp position minus the ambu start, used for volume calculation
	}

	//stepPosIns must be at least the STEPS_SWITCH_LIMIT_OFFSET value
	if(ROTATIONAL_DIRECTION*gMotorParameters.cycles[cycleType].stepPosIns < STEPS_SWITCH_LIMIT_OFFSET)
		gMotorParameters.cycles[cycleType].stepPosIns = ROTATIONAL_DIRECTION*STEPS_SWITCH_LIMIT_OFFSET;

	//gMotorParameters.cycles[cycleType].stepPosIns += STEPS_SWITCH_LIMIT_OFFSET; //Note that we do not need to multiply by ROTATIONAL_DIRECTION, since endLimitSwitchPos inverts for insp/exp already

	long breathStepsPerInsOrExp = ROTATIONAL_DIRECTION*(gMotorParameters.cycles[cycleType].stepPosExp - gMotorParameters.cycles[cycleType].stepPosIns);


	//calculate the stepsPerSecond according to bpm (ventilation per minute)
	//stepsPerSecond = (steps per minute)/60
	float stepsPerSecondPerInsOrExp = (gActiveRespCfg.bpm * breathStepsPerInsOrExp)/(float)60;

	//Note that both insp. and expiration will have the same amount of steps
	//inspiration must perform those steps FASTER than expiration, in order to
	//do it at a faster ratio

	//Now, we have the ratio between inpiration and expiration
	//If 1:1, 50% of the TIME will be for each side
	//If 1:2, 1/3 will be insp. and 2/3 will be exp, thus:

	float timePerInsAndExp = 60/(float)gActiveRespCfg.bpm;
	float insTime = (timePerInsAndExp/(1 + gActiveRespCfg.ie_ratio)); //without TOTAL_MOTOR_WAIT_PERIOD
	float expTime = ((insTime *  gActiveRespCfg.ie_ratio)/((float)SET_EXPIRATION_SPEED));

	float waitPeriod = 0.001f * TOTAL_MOTOR_WAIT_PERIOD;
	if(waitPeriod > insTime*MAX_WAIT_PERIOD_FRAC_INSP)
		waitPeriod = insTime*MAX_WAIT_PERIOD_FRAC_INSP;

	//The expiration must do breathStepsPerInsOrExp AVERAGE per second,
	//But it will have only ratio/(1+ratio) of the total time do do it
	//thus, it must multiply stepsPerSecondPerInsOrExp by (1+ratio)/ratio.
	//*(1000*expTime/(1000*expTime-TOTAL_MOTOR_WAIT_PERIOD))
	gMotorParameters.cycles[cycleType].expVel = SET_EXPIRATION_SPEED * stepsPerSecondPerInsOrExp * (((1 + gActiveRespCfg.ie_ratio)/gActiveRespCfg.ie_ratio));

	//after ther expTime calculation, we must substract the insTime so the time used to move does not consider
	insTime -= waitPeriod;

	//The inspiration must do breathStepsPerInsOrExp steps in insTime seconds.
	//Velocity (steps/s) = Steps/Time
	gMotorParameters.cycles[cycleType].insVel = breathStepsPerInsOrExp/insTime;

	#if MOTOR_ACCELERATION > 0
		#ifndef MOTOR_PWM
		  //The motor must accelerate in 1/x of the time
		  //the final speed must be adjusted in order to compensate the time lost in the acceleration/deacceleration proccess.
		  gMotorParameters.cycles[cycleType].insVel *= 1.02f * MOTOR_ACCELERATION/(double)(MOTOR_ACCELERATION-1);
		  gMotorParameters.cycles[cycleType].expVel *= 1.02f * MOTOR_ACCELERATION/(double)(MOTOR_ACCELERATION-1);
		/*
		  //if there a fixed expiration phase speed?
		  #if SET_EXPIRATION_SPEED > 0
			//the SET_EXPIRATION_SPEED is higher than the calculated one?
			if(SET_EXPIRATION_SPEED > gMotorParameters.cycles[cycleType].expVel)
			  gMotorParameters.cycles[cycleType].expVel = SET_EXPIRATION_SPEED;
		  #endif*/


		  //the acceleration must be set so that it reachs the full speed in time/x
		  gMotorParameters.cycles[cycleType].insAcel = gMotorParameters.cycles[cycleType].insVel/(insTime/MOTOR_ACCELERATION);
		  gMotorParameters.cycles[cycleType].expAcel = gMotorParameters.cycles[cycleType].expVel/(expTime/MOTOR_ACCELERATION);
		#endif
	#endif

	#ifdef MOTOR_PWM
		//if there a fixed expiration phase speed?
		/*
		#if SET_EXPIRATION_SPEED > 0
		  //the SET_EXPIRATION_SPEED is higher than the calculated one?
		  if(SET_EXPIRATION_SPEED > gMotorParameters.cycles[cycleType].expVel)
			gMotorParameters.cycles[cycleType].expVel = SET_EXPIRATION_SPEED;
		#endif
		*/

		#if MOTOR_ACCELERATION > 0
		  //first we must find the frequency for the speed without acceleration
		  //note that this calculation will be done again after we find the final desired speed
		  gMotorParameters.cycles[cycleType].insTargetSpeedFreq = findFreqSpeed(gMotorParameters.cycles[cycleType].insVel, &gMotorParameters.cycles[cycleType].insPWMPrescaler);
		  gMotorParameters.cycles[cycleType].expTargetSpeedFreq = findFreqSpeed(gMotorParameters.cycles[cycleType].expVel, &gMotorParameters.cycles[cycleType].expPWMPrescaler);

		  int targetFreq = ((int)gMotorParameters.cycles[cycleType].insTargetSpeedFreq)+MAX_RANGE_PWM_VALUE;
		  //in order to obtain more accurate values, we save the start freq speed
		  if(targetFreq < MAX_PWM_VALUE)
			gMotorParameters.cycles[cycleType].insStartSpeedFreq = targetFreq;
		  else
			gMotorParameters.cycles[cycleType].insStartSpeedFreq = MAX_PWM_VALUE;

		  targetFreq = ((int)gMotorParameters.cycles[cycleType].expTargetSpeedFreq)+MAX_RANGE_PWM_VALUE;
		  if(targetFreq < MAX_PWM_VALUE)
			gMotorParameters.cycles[cycleType].expStartSpeedFreq = targetFreq;
		  else
			gMotorParameters.cycles[cycleType].expStartSpeedFreq = MAX_PWM_VALUE;

		  #ifndef ACCELERATE_EVERY_PULSE

			//calculate the mean velocity for the acceleration stage
			//If there is an acceleration, we must now find the speed for the start acceleration
			float meanInsAccelSpeed = calcMeanSpeed(gMotorParameters.cycles[cycleType].insStartSpeedFreq-1, gMotorParameters.cycles[cycleType].insTargetSpeedFreq-1, gMotorParameters.cycles[cycleType].insPWMPrescaler);///2;
			float meanExpAccelSpeed = calcMeanSpeed(gMotorParameters.cycles[cycleType].expStartSpeedFreq-1, gMotorParameters.cycles[cycleType].expTargetSpeedFreq-1, gMotorParameters.cycles[cycleType].expPWMPrescaler);///2;

			//we can now calculate the desired final speed to compensate acceleration
			//the final speed must be adjusted in order to compensate the time lost in the acceleration/deacceleration proccess.
			gMotorParameters.cycles[cycleType].insVel = ((gMotorParameters.cycles[cycleType].insVel * MOTOR_ACCELERATION)-(2*meanInsAccelSpeed))/(double)(MOTOR_ACCELERATION-2);
			gMotorParameters.cycles[cycleType].expVel = ((gMotorParameters.cycles[cycleType].expVel * MOTOR_ACCELERATION)-(2*meanExpAccelSpeed))/(double)(MOTOR_ACCELERATION-2);
		  #else
			float meanAccelSpeed, accelTime;
			calcMeanSpeed(&meanAccelSpeed, &accelTime, &gMotorParameters.cycles[cycleType].insStartSpeedFreq, &gMotorParameters.cycles[cycleType].insTargetSpeedFreq, insTime, gMotorParameters.cycles[cycleType].insPWMPrescaler);
			if(accelTime != 0)
			  gMotorParameters.cycles[cycleType].insVel = ((2*meanAccelSpeed*accelTime) - (insTime*gMotorParameters.cycles[cycleType].insVel))/((2*accelTime)-insTime);

			calcMeanSpeed(&meanAccelSpeed, &accelTime, &gMotorParameters.cycles[cycleType].expStartSpeedFreq, &gMotorParameters.cycles[cycleType].expTargetSpeedFreq, expTime, gMotorParameters.cycles[cycleType].expPWMPrescaler);
			if(accelTime != 0)
			  gMotorParameters.cycles[cycleType].expVel = ((2*meanAccelSpeed*accelTime) - (expTime*gMotorParameters.cycles[cycleType].expVel))/((2*accelTime)-expTime);
		  #endif
		#endif

		gMotorParameters.cycles[cycleType].insTargetSpeedFreq = findFreqSpeed(gMotorParameters.cycles[cycleType].insVel, &gMotorParameters.cycles[cycleType].insPWMPrescaler);
		gMotorParameters.cycles[cycleType].expTargetSpeedFreq = findFreqSpeed(gMotorParameters.cycles[cycleType].expVel, &gMotorParameters.cycles[cycleType].expPWMPrescaler);

		//microTimes are only used when we are not accelerating every pulse
		#ifndef ACCELERATE_EVERY_PULSE
		  gMotorParameters.cycles[cycleType].insMicroTime = insTime  * 1000000; // *1000000 to convert to microsec
		  gMotorParameters.cycles[cycleType].expMicroTime = expTime  * 1000000; // *1000000 to convert to microsec
		  #if MOTOR_ACCELERATION > 0
			gMotorParameters.cycles[cycleType].insMicroAcelTime = (insTime/MOTOR_ACCELERATION) * 1000000;
			gMotorParameters.cycles[cycleType].expMicroAcelTime = (expTime/MOTOR_ACCELERATION) * 1000000;
		  #else
			gMotorParameters.cycles[cycleType].insMicroAcelTime = 0;
			gMotorParameters.cycles[cycleType].expMicroAcelTime = 0;
		  #endif
		#endif

	#endif


	#if DEBUG_CODE == 1
		//txSerial.print(F("insTime: "));
		//txSerial.println(insTime);
		/*
		txSerial.print(F("breathStepsPerInsOrExp: "));
		txSerial.println(breathStepsPerInsOrExp);*/
	#ifndef MOTOR_PWM
	  txSerial.print(F("gM.insA: "));
	  txSerial.println(gMotorParameters.cycles[cycleType].insAcel);
	#endif/*
		txSerial.print(F("stepsPerSecond: "));
		txSerial.println(stepsPerSecondPerInsOrExp);*/
		//txSerial.println(F("I P,V; E P,V:"));
		txSerial.println(F("I P, E P:"));
		txSerial.println(gMotorParameters.cycles[cycleType].stepPosIns);
		//txSerial.print(F(", v:"));
		//txSerial.println(gMotorParameters.cycles[cycleType].insVel);
		//txSerial.print(F("Exp p:"));
		txSerial.println(gMotorParameters.cycles[cycleType].stepPosExp);
		//txSerial.print(F(", v:"));
		//txSerial.println(gMotorParameters.cycles[cycleType].expVel);
	#endif
}

//Update the motor param. if
void calculateMotorParam(bool forceUpdate)
{
  //Should we update the motor parameters?
  if(forceUpdate || gActiveRespEst.newMotorConfig == true)
  {
		//ensure that we know the current cycle bpm
		gActiveRespEst.cycle_bpm = gActiveRespCfg.bpm;
		//calculate the normal cycle parameters
		calculateCycleParam(NORMAL_CYCLE);
		//calculate the test cycle parameters
		calculateCycleParam(TEST_CYCLE);

		gActiveRespEst.newMotorConfig = false;
  }
}

void turnOffScreen()
{
	//this function may only be called at S2 state
	if(getSystemState() != S2)
		return;
	CmdIhm checkVal = NONE;
	do
	{
		//show the turn off screen
		checkVal = printMessageLinesWaitResp("X00Y:",5);//message codes
		//Ok means turn off
		if(checkVal == CMD_OK)
		{
			resetAllAlarms();
			//update the watchdog flag
			gActiveRespCfg.restartMethod = RESTART_NORMAL;
			//save the updated configuration
			saveRespConfiguration(&gActiveRespCfg);
			//call the last message function (blocking)
			printLastMessage(MSG_SYSTEM_OFF, false);
		}
	} while(checkVal != CONFIG);

	//delay any further button press.
	delayButtonCheck(800);
	//any other button means cancel
	requestScreenUpdate();
	return;
}
//Stopped state
State funcS2()
{
  CmdIhm checkVal = NONE;
  unsigned long now;
  unsigned long lastUpdate = 0;

  //make sure the watchdog is started.
  if(!getWatchdogStatus())
	watchdog_init(&gActiveRespCfg);

  //calculate the respiration parameters
  //not that this is necessary in order to go to the stepPosExp
  calculateMotorParam(true); //force update

  //Go to expiration position if we are not in the stand by state.
  if(!gActiveRespEst.standBy)
  	//note that the first parameter must be 0, since we are going to the inspiration direction to reach stepPosExp.
  	goToPositionAndFindSwitch(0, gMotorParameters.cycles[NORMAL_CYCLE].stepPosExp, NULL);


  // Wait Confirmation form IHM
  while(checkVal != CMD_OK) //did the user confirm the cfg?
  {
    now = millis();
    if(now - lastUpdate > 2000) //2s
    {
		lastUpdate = now;
		requestScreenUpdate();
	}
	//The system is paused. Watchdog reset must be called allways.
	watchdog_reset();
    checkVal = ihm_check();
    //is there any changes in the active configuration?
    if(checkVal == NEWDATA)
      saveRespConfiguration(&gActiveRespCfg);
    else if(checkVal == CMD_ALR) //alarm button pressed
      alarmButtonPress();
    else if(checkVal == CMD_STAND_BY) //is the user trying to shut down the equipment?
      turnOffScreen();
    //update beep
	if(timer1_beepIsRunning())
    	timer1_beep_update();
  }

  //make sure we update the stand-by flag.
#ifndef STAND_BY_DEBUG
	gActiveRespEst.standBy = false;
#endif

	//Requests the system to update the motor parameters
	gActiveRespEst.newMotorConfig = true;

//  #if DEBUG_CODE == 1
//    txSerial.println(F("v,b,ie:"));
//    txSerial.println(gActiveRespCfg.volume_pulses);
//    txSerial.println((int)gActiveRespCfg.bpm);
//    txSerial.println((int)gActiveRespCfg.ie_ratio);
//  #endif
  return S3;
}
