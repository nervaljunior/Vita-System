#include "alarm.h"
#include "timer1.h"
#include "global.h"
#include "configmanager.h"
#include "motor.h"
#include "ihm.h"
#include "pressure.h"
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

CycleParameters *configureExpMotor(bool goToEndLimitSwitch)
{
  CycleParameters *cycleParam;
  //if we are at standby status or if we are at a TEST_CYCLE, we must completely open.
  if(goToEndLimitSwitch)
	cycleParam = &gMotorParameters.cycles[TEST_CYCLE];
  else
	cycleParam = &gMotorParameters.cycles[NORMAL_CYCLE];

  #if DEBUG_CODE == 7
    if(goToEndLimitSwitch)
    	txSerial.print(F("CY "));
    txSerial.println(F("E C e T Pos: "));
    txSerial.println(motorGetPosition());
    txSerial.println(cycleParam->stepPosExp);
  #endif

#ifndef MOTOR_PWM
  motorMoveTo(cycleParam->stepPosExp, EXPIRATION_SIGN, cycleParam->expVel, cycleParam->expAcel);
#else
  #ifndef ACCELERATE_EVERY_PULSE
    motorPWMMoveTo(cycleParam->stepPosExp, EXPIRATION_SIGN, cycleParam->expTargetSpeedFreq, cycleParam->expStartSpeedFreq, cycleParam->expMicroTime, cycleParam->expMicroAcelTime, cycleParam->expPWMPrescaler);
  #else
    motorPWMMoveTo(cycleParam->stepPosExp, EXPIRATION_SIGN, cycleParam->expTargetSpeedFreq, cycleParam->expStartSpeedFreq, cycleParam->expPWMPrescaler);
  #endif
#endif
	return cycleParam;
}


State verifyStartBreath(State oldState)
{
  State state;
  static float historicalPEEP = 0;
  unsigned long now;
  bool peepObtained = false;
  bool firstTimeOfThisCycle = true;
#ifdef USE_PRESSURE_SENSOR
  float sensorSample;
  PeepInfo peepInfo = NO_INFO;
  bool assist_cur_breath = false;
  PSensorTrigger pSensorTrigger = TRIGGERNONE;
#endif //USE_PRESSURE_SENSOR

  //sice we must calculate the remaning time using the current cycle bpm
  //even if the configuration is changed, we must guarantee that cycle_bpm
  //has a valid value
  if(gActiveRespEst.cycle_bpm < MIN_BPM || gActiveRespEst.cycle_bpm > MAX_BPM)
  {
	  //if so, reset to the bpm config. value.
	  gActiveRespEst.cycle_bpm = gActiveRespCfg.bpm;
	  //since it should not happen, set an alarm.
	  setAlarm(ALARM_G);
  }

  //compute the differences at the start of the next breath
  if(gInitialBreathTime > 0)
  {
	//one last watchdog reset call before the while.
	watchdog_reset();

    //we should now be sure that we are not too fast
    while(1)
    {
		//Do NOT call watchdog_reset() in here.
		//verifyStartBreath should finish before the watchdog expiration.
		//WARNING: if SET_EXPIRATION_SPEED is changed, one might have to increase the watchdog reset time since breath and plateau procedures do not call watchdog_reset.

      	now = millis();


#ifdef USE_PRESSURE_SENSOR
      //obtain the PEEP
      if(!peepObtained && peepInfo != PEEPVOLATILE)
      {
        //try to obtain the PEEP
        peepInfo = timer1_pSensor_calcPeep(firstTimeOfThisCycle,&sensorSample);
        firstTimeOfThisCycle = false;
        //did it fishing the PEEP acquisition?
        if(peepInfo == PEEPVALID){
          gActiveRespEst.peep_min_mea = timer1_pSensor_getPeep();
          peepObtained = true;
#if DEBUG_CODE == 8
		  //For serial plot
          txSerial.println((int)gActiveRespEst.peep_min_mea);
#endif
		  gActiveRespEst.peep_min_mea = ROUND_DOT5_FLOAT(gActiveRespEst.peep_min_mea);

          // make sure we wont consider an invalid PEEP
          if(historicalPEEP == 0)
            historicalPEEP = gActiveRespEst.peep_min_mea;
          // verify is PEEP isn't bellow the minimum value set
          if(gActiveRespEst.peep_min_mea < gActiveRespCfg.peep_min && gActiveRespCfg.op_mode != N_MODE)
            setAlarm(ALARM_F);
        } else if(peepInfo == PEEPVOLATILE && gActiveRespCfg.op_mode != N_MODE) {
		  setAlarm(ALARM_D);
        }
      }

      //if we were able to obtain the PEEP and there is a new sample available
      if((peepObtained || peepInfo == PEEPVOLATILE) && TRIGGERNONE==pSensorTrigger)
      {

  		 PSensorTrigger pSensorTrigger = timer1_pSensor_trigger(gActiveRespCfg.trigger);

  		 if(pSensorTrigger == TRIGGERINSPIRATION &&
  			gActiveRespCfg.op_mode == AC_MODE){
  			assist_cur_breath = true;
  			break;
  		 }else if(TRIGGERFAULT == pSensorTrigger && gActiveRespCfg.op_mode != N_MODE){
  			//TODO: Check trigger Alarm
  			// TODO: turn alarm on
  			// Initiate by time
  			//TODO: Check if this alarm is OK
  			setAlarm(ALARM_D);
  		 }
      }
#endif //USE_PRESSURE_SENSOR

      //TODO: Reread this function to ensure it will eventually exit the loop
      //-1 millis to allow the software to reach the inspiration phase again in time
      if((unsigned long)(now - gInitialBreathTime) > (unsigned long)((1000*(60/(float)gActiveRespCfg.bpm))-1))
        break;

      //Verify overflow
      if(now < gInitialBreathTime)
        break;

      state = verifyIHM(oldState);//, EXPIRATION_SIGN);
      if(state != NA)
        return state;
    } //while(1)

	//one watchdog reset call after the while.
	watchdog_reset();

#ifdef USE_PRESSURE_SENSOR
    //update the assist_cur_breath
    gActiveRespEst.assist_cur_breath = assist_cur_breath;

    if(peepObtained)
    {
        //If PEEP dropped too much in comparasion with historical values
        if(gActiveRespEst.peep_min_mea < historicalPEEP - MAXIMUM_ALLOWABLE_PEEP_DROP && gActiveRespCfg.op_mode != N_MODE)
          setAlarm(ALARM_C); //

        //TODO: Evaluate if we must perform a better estimative of the historical PEEP
        historicalPEEP = (historicalPEEP + gActiveRespEst.peep_min_mea)/2;
    }
#endif //USE_PRESSURE_SENSOR
  }

  return NA;
}
#ifdef USE_PRESSURE_SENSOR
inline void updatePIP()
{
  static float historicalPIP = 0;
  //PIP is saved at the begging of expiration process (plateau period)
  //save the PIP
  gActiveRespEst.pip_max_mea = gActiveRespEst.maxCyclePressure;
  gActiveRespEst.pip_max_mea = ROUND_DOT5_FLOAT(gActiveRespEst.pip_max_mea);
  if(historicalPIP == 0)
    historicalPIP = gActiveRespEst.pip_max_mea;

  //if the PIP is bellow the minimum allowable PIP
  if(gActiveRespEst.pip_max_mea < gActiveRespEst.peep_min_mea + MINIMUM_PIP_INCREASE && gActiveRespCfg.op_mode != N_MODE)
  {
    setAlarm(ALARM_B);
  } //if the PIP is way bellow the historical PIP valeus
  else if(gActiveRespEst.pip_max_mea < historicalPIP - MAXIMUM_ALLOWABLE_PIP_DROP && gActiveRespCfg.op_mode != N_MODE)
  {
    setAlarm(ALARM_C);
  }

  //update the historical value
  historicalPIP = (historicalPIP + gActiveRespEst.pip_max_mea)/2; //TODO: Evaluate if we must perform a better estimative of the historical PIP

  //Update screen
  requestScreenUpdate();

#if DEBUG_CODE == 8
	//For serial plot
	txSerial.print((int)gActiveRespEst.pip_max_mea);
	txSerial.print(" ");
#endif
}
#endif
//Expiration state
State funcS4(State oldState)
{
	State state = NA;
	bool endLimitReached = false;

	//make sure the watchdog is started.
	if(!getWatchdogStatus())
		watchdog_init(&gActiveRespCfg);


	//Should we do a test cycle or are we at stand by procedure?
	if(gActiveRespEst.standBy || gActiveRespEst.currentCycle == 0)
	{
		//do not reset current cycle yet
		//it must be done after the while below
		configureExpMotor(true);
	} else {
		configureExpMotor(false);
	}

	//note that the gMotorParameters.stepPosIns can be changed inside the while
	long lastDistanceToGo = motorDistanceToGo(), currentDistanceToGo;
	while(motorDistanceToGo() != 0)
	{
		currentDistanceToGo = motorDistanceToGo();
		//we should update the watchdog only if the distance to go is changing.
		if(lastDistanceToGo != currentDistanceToGo)
		{
			lastDistanceToGo = currentDistanceToGo;
			watchdog_reset();
		}
		//TODO: REMOVE THE 2 LINES BELOW.
		//if(getMenuState() == GRAPH_SCREEN)
		//	delay(7000); //force reset

#ifdef USE_PRESSURE_SENSOR
		updateMaxPressure();
#endif
		//verify if we reached the end limit switch
		if(digitalRead(LIMIT_SWITCH_END_PIN) == LIMIT_SWITCH_REACHED)
		{
			endLimitReached = true;
			#if DEBUG_CODE == 1 || DEBUG_CODE == 7
			  txSerial.print(F("Ex L Re "));
			  txSerial.println(motorGetPosition());
			#endif
			//we did reach the end limit switch
			//set the current position that correponds to the end limit switch
			motorSetPosition(gActiveRespCfg.endLimitSwitchPos);
			motorStop();
			//exit the while
			break;
		}
		motorRun();

		state = verifyIHM(oldState);//, EXPIRATION_SIGN);
		if(state != NA)
			return state;
	}
	motorStop();
	motorUpdatePosition();

	//make sure we update the endLimitReached flag.
	if(digitalRead(LIMIT_SWITCH_END_PIN) == LIMIT_SWITCH_REACHED)
		endLimitReached = true;

	//we did reach the end limit
	if(endLimitReached)
	{
		//is it because we requested a test cycle?
		if(gActiveRespEst.currentCycle == 0)
		  //reset the current cycle, everything is ok.
		  gActiveRespEst.currentCycle = CYCLES_PER_MOTOR_TEST;
		else //the stand by button might be the cause or it results in an alarm B
		{
		  //if we are not in the standBy procedure
		  if(!gActiveRespEst.standBy)
		  {
			//we are missing too many steps
			setAlarm(ALARM_B);
		  }
		  //the current cycle must be decremented anyway in this case
		  gActiveRespEst.currentCycle--;
		}
	} else {
		//should it had happen?
		if(gActiveRespEst.currentCycle == 0)
		{
			//we will set the alarm and DO NOT update the current cycle, making it to perform a test again next cycle
			setAlarm(ALARM_A);
			//Also, in order to be able to reach the end limit switch next time, we must also reset our current position
			//to endLimitSwitchPos. Note that this will make that, in the next attempt, the system will go futher
			//STEPS_TEST_CYCLE_INCREASE than the current position
			motorSetPosition(gActiveRespCfg.endLimitSwitchPos);
		} else {
			gActiveRespEst.currentCycle--;
		}
	}
#ifdef USE_PRESSURE_SENSOR
  //Update PIP value
  updatePIP();
#endif

//#ifdef RESET_LCD_OFTEN
// resetLCD(); //reset LCD to avoid any possible problems
//#endif

  state = verifyStartBreath(oldState);
  if(state != NA)
    return state;

  //the stand-by status makes the system to completely open and stop.
  if(gActiveRespEst.standBy)
  {
	gActiveRespEst.volume_est = 0;
	gActiveRespEst.bpm_mea = 0;
	gActiveRespEst.ie_ratio_mea = 0;
	gActiveRespEst.flux_est = 0;
#ifdef RESET_LCD_OFTEN
		printMessageLine(MSG_PAUSED_SYSTEM);
		delay(1500); //so that the user can see the message
		//if KEY_STAND_BY was pressed
  		resetLCD();
#endif
	//Update screen
	requestScreenUpdate();
	return S2;
  }
  else
    return S3;
}
