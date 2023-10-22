#include "alarm.h"
#include "global.h"
#include "configmanager.h"
#include "motor.h"
#include "timer1.h"
#include "ihm.h"
#include "pressure.h"
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

#ifdef USE_PRESSURE_SENSOR
//Used to maintain the max pressure of each cycle.
void updateMaxPressure()
{
	//note that gActiveRespEst.maxCyclePressure is reset at the beginning of S3.
	float sensorSample;

	//verify inspiration pressure
	if(timer1_pSensor_newSampleAvailable())
	{
	  //read the sample
	  sensorSample = timer1_pSensor_read_cmH2O();
	  if(gActiveRespEst.maxCyclePressure < sensorSample)
		gActiveRespEst.maxCyclePressure = sensorSample;
	}
}
#endif

CycleParameters *configureInspMotor()
{
	CycleParameters *cycleParam;
	if(ROTATIONAL_DIRECTION*motorGetPosition() >= ROTATIONAL_DIRECTION*gActiveRespCfg.endLimitSwitchPos)
		cycleParam = &gMotorParameters.cycles[TEST_CYCLE];
	else
		cycleParam = &gMotorParameters.cycles[NORMAL_CYCLE];

#if DEBUG_CODE == 7
    txSerial.println(F("I C e T Pos: "));
    txSerial.println(motorGetPosition());
    txSerial.println(cycleParam->stepPosIns);
#endif

#ifndef MOTOR_PWM
  motorMoveTo(cycleParam->stepPosIns, INSPIRATION_SIGN, cycleParam->insVel, cycleParam->insAcel);
#else
	#ifndef ACCELERATE_EVERY_PULSE
		motorPWMMoveTo(cycleParam->stepPosIns, INSPIRATION_SIGN, cycleParam->insTargetSpeedFreq, cycleParam->insStartSpeedFreq, cycleParam->insMicroTime, cycleParam->insMicroAcelTime, cycleParam->insPWMPrescaler);
	#else
		motorPWMMoveTo(cycleParam->stepPosIns, INSPIRATION_SIGN, cycleParam->insTargetSpeedFreq, cycleParam->insStartSpeedFreq, cycleParam->insPWMPrescaler);
	#endif
#endif
	return cycleParam;
}

State verifyIHM(State oldState)//, InsExpSign phase)
{
  CmdIhm checkVal;
  // Wait Confirmation form IHM
  checkVal = ihm_check();

  //is there any changes in the active configuration?
  if(checkVal == NEWDATA)
  {
	//delay any further button press.
	delayButtonCheck(1000);
    //save the configuration in the EEPROM
    saveRespConfiguration(&gActiveRespCfg);
    //Requests the system to update the motor parameter
    gActiveRespEst.newMotorConfig = true;

  } else if(checkVal == CMD_STAND_BY && (oldState == S4 || oldState == S3))
  {
	//delay any further button press.
	delayButtonCheck(1500);
	//if we are not in stand-by status already.
    if(!gActiveRespEst.standBy)
    {
	   //we should enter the stand-by status
		gActiveRespEst.standBy = true;
		//we must go to a new expiration states (even if we are already in it)
		//to be able to set the final position to maximum avaliable position.
        return S4;
	}
  } else if(checkVal == CMD_ALR) //alarm button pressed
  {
    alarmButtonPress();
  }
  //update beep
  if(timer1_beepIsRunning())
	timer1_beep_update();

  return NA;
}
State plateauMeasurements(State oldState)
{
  unsigned long initialPlateauTime = millis(), now;
  State state;

  do
  {
	//plateau perid should NOT update watchdog, sice it must finish before the watchdog timer limit
	//Do NOT call watchdog_reset() in here.
	//WARNING: if SET_EXPIRATION_SPEED is changed, one might have to increase the watchdog reset time since breath and plateau procedures do not call watchdog_reset.

#ifdef USE_PRESSURE_SENSOR
	updateMaxPressure();
    //TODO: Obtain the plateau?
#endif
	  //Is it time to start the expiration phase?
	  //-1 millis to allow the software to reach the inspiration phase again in time
	  if((unsigned long)(millis() - gInitialBreathTime) > (unsigned long)((1000*((60/(float)gActiveRespCfg.bpm)/(1+gActiveRespCfg.ie_ratio)))-1))
	  {
		#if DEBUG_CODE == 9
    		txSerial.println(millis() - gInitialBreathTime);
    	#endif

		break;
	  }

	//Verify IHM
	state = verifyIHM(oldState);//, EXPIRATION_SIGN);
	if(state != NA)
		return state;



	now = millis();
  //The second expression is to verify possible overflow
  } while(now < (unsigned long)(initialPlateauTime + PLATEAU_PERIOD_MS) && (unsigned long)(now+1) > initialPlateauTime);

  return NA;
}


//Inspiration state
State funcS3(State oldState)
{
  State state = NA , returnState = S4;

#ifdef USE_PRESSURE_SENSOR
  //reset the max cycle pressure every cycle
  gActiveRespEst.maxCyclePressure = -MAX_PIP;
#endif

  //make sure the watchdog is started.
  if(!getWatchdogStatus())
	watchdog_init(&gActiveRespCfg);

  //calculate the respiration parameters
  calculateMotorParam(false); //update only if necessary

  //Configure the motor and obtain the cycle params. used;
  CycleParameters *cycleParam = configureInspMotor();

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

    //if there is a start limit switch
    #ifdef START_LIMIT_SWITCH
      //verify if we reached it
      if(digitalRead(LIMIT_SWITCH_INI_PIN) == LIMIT_SWITCH_REACHED)
      {
		#if DEBUG_CODE == 1 || DEBUG_CODE == 7
          txSerial.print(F("In L Re "));
		  txSerial.println(motorGetPosition());
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
    state = verifyIHM(oldState);//, INSPIRATION_SIGN);
    if(state != NA)
    {
      returnState = state;
      break;
    }
#ifdef USE_PRESSURE_SENSOR
    //update the max. pressure of this cycle
	updateMaxPressure();

	// is the current pressure above the setted threshold?
	if(gActiveRespEst.maxCyclePressure > gActiveRespCfg.pip_max && gActiveRespCfg.op_mode != N_MODE)
	{
		//set alarm and stop the inspiration proccess
		setAlarm(ALARM_E);
		break; //DO NOT return here. The motor must also be stoped after the while.
	}
#endif
  }
  motorStop();
  motorUpdatePosition();

  //update the volume and flux
  //gActiveRespEst.volume_est = 100*(gMotorParameters.stepPosExp - motorGetPosition())/(gMotorParameters.stepPosExp-(ROTATIONAL_DIRECTION*STEPS_SWITCH_LIMIT_OFFSET));
  gActiveRespEst.volume_est = ROTATIONAL_DIRECTION*(cycleParam->posExpVol - motorGetPosition());
  // Conversion to ML if macro VOLUME_USE_ML is defined
  gActiveRespEst.volume_est = VOLUME_CALC(gActiveRespEst.volume_est);

  //Do not allow negative values
  if(gActiveRespEst.volume_est < 0)
  	gActiveRespEst.volume_est = 0;

  gActiveRespEst.flux_est = (1+gActiveRespEst.ie_ratio_mea)*(gActiveRespEst.volume_est * gActiveRespEst.bpm_mea)/1000.0f;

  //Note that PIP is calculated at the beginning of S4 function.

  //Update screen
  requestScreenUpdate();

	//obtain the plateau in the plat. period.
	state = plateauMeasurements(oldState);
	if(state != NA)
		return state;

  return returnState;
}
