#include "alarm.h"
#include "global.h"
#include "ihm.h"
#include "motor.h"
#include "timer1.h"
#include "configmanager.h"
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

void systemFail(int msg)
{
	gActiveRespCfg.faultDetected = true;
	//save the updated configuration
	saveRespConfiguration(&gActiveRespCfg);
	//something went wrong. Stopping the proccess
	printLastMessage(msg, false);
}
void performStartProcedure(bool hiddenConfig, int totalNumberTests)
{
	#ifdef START_LIMIT_SWITCH
		double ratioCount;
		float meanInsp, stdDevInsp, stdErrorInsp;
		float meanExp, stdDevExp, stdErrorExp;
		long measuresInsp[HIDDEN_CONFIG_NUM_TESTS];
		long measuresExp[HIDDEN_CONFIG_NUM_TESTS];

		if(totalNumberTests > HIDDEN_CONFIG_NUM_TESTS)
			totalNumberTests = HIDDEN_CONFIG_NUM_TESTS;

		//first, we must reach the end limit switch
		if(moveStepsAndFindSwitch(1, MAXIMUM_START_SEQ_NUM_STEPS) == false)
		{
			//something went wrong. Stopping the proccess
			systemFail(MSG_NO_END_LIMIT_SWITCH);
		}

		for(int i = 0; i < totalNumberTests; i++)
		{
			// zero out the current position
			motorSetPosition(0);

			// necessary even if there is no pressure sensor
			// to allow the motor to run properly.
			delay(END_INSP_DELAY_MS);

			//go to the inspiration switch
			if(moveStepsAndFindSwitch(0, MAXIMUM_START_SEQ_NUM_STEPS) == false)
			{
				//something went wrong. Stopping the proccess
				systemFail(MSG_NO_START_LIMIT_SWITCH);
			}
			//obtain the first count (might be positive or negative)
			measuresInsp[i] = motorGetPosition();
			//zero the value again
			motorSetPosition(0);

			// necessary even if there is no pressure sensor
			// to allow the motor to run properly.
			delay(END_INSP_DELAY_MS);

			//second, we must reach the inspiration switch
			if(moveStepsAndFindSwitch(1, MAXIMUM_START_SEQ_NUM_STEPS) == false)
			{
				//something went wrong. Stopping the proccess
				systemFail(MSG_NO_END_LIMIT_SWITCH);
			}
			//obtain the first count (might be positive or negative)
			measuresExp[i] = motorGetPosition();
		}
		//
		calcSampleMeanStdDevError(measuresInsp, totalNumberTests, &meanInsp, &stdDevInsp, &stdErrorInsp);
		calcSampleMeanStdDevError(measuresExp, totalNumberTests, &meanExp, &stdDevExp, &stdErrorExp);

		#if DEBUG_CODE == 5
      		//txSerial.println(F("I,E(MSE) "));
    		txSerial.println(meanInsp);
    		txSerial.println(stdDevInsp);
    		txSerial.println(stdErrorInsp);
      		//txSerial.println(F("E M,S,E "));
    		txSerial.println(meanExp);
    		txSerial.println(stdDevExp);
    		txSerial.println(stdErrorExp);
		#endif

		if(!hiddenConfig)
		{
			//determine the ratio between inspiration and expiration
			//note that, size they went in opose directions, the value will be negative
			ratioCount = -1.0f*(((double)meanInsp)/meanExp);
			if(ratioCount > (1.0f+MAX_ALLOWABLE_STEP_ERROR) || ratioCount < (1.0f-MAX_ALLOWABLE_STEP_ERROR))
			{
				gActiveRespCfg.faultDetected = true;
				//save the updated configuration
				saveRespConfiguration(&gActiveRespCfg);
			#if DEBUG_CODE != 0 && DEBUG_CODE != 5 && DEBUG_CODE != 8
				#if DEBUG_CODE == 1
					//txSerial.print(F("Err% "));
					//txSerial.println(ratioCount);
				#endif
				#ifdef LCD_128x64
					printMessageLine(MSG_STEP_LOSS_ERROR);
				#endif
				delay(2000); //so that the user can see the message
			#else
				systemFail(MSG_STEP_LOSS_ERROR); //this function will stop the process
			#endif
			}
		} else {
			//we set the current position to that which represents the end limit swicht position
			//might be positive or negative, dependending on the ROTATIONAL_DIRECTION define.
			//TODO: use the both, insp or exp mean?
			//gActiveRespCfg.endLimitSwitchPos = (((-1)*meanInsp)+meanExp)/2;
			gActiveRespCfg.endLimitSwitchPos = meanExp + (ROTATIONAL_DIRECTION*3*stdDevExp); //99.7% of the data is within 3 standard deviations
		}
	#else
		gActiveRespCfg.endLimitSwitchPos = ROTATIONAL_DIRECTION*DEFAULT_NUMBER_STEPS;

		if(moveStepsAndFindSwitch(1, MAXIMUM_START_SEQ_NUM_STEPS) == false)
		{
		  //something went wrong. Stopping the proccess
		  printLastMessage(MSG_NO_END_LIMIT_SWITCH, false);
		}
	#endif
	//motorSetPosition(gActiveRespCfg.endLimitSwitchPos); //not needed, since endLimitSwitchPos = secondCount = motorGetPosition()
	/*#if DEBUG_CODE == 5
	  txSerial.print(F("E L "));
	  txSerial.println(gActiveRespCfg.endLimitSwitchPos);
	#endif*/

	//we set the current position to that which represents the end limit swicht position
	motorSetPosition(gActiveRespCfg.endLimitSwitchPos);
	motorStop();
}

bool moveStepsAndFindSwitch(int endSwitch, long numSteps, float *maxRawADCPressure)
{
	long finalPosition = motorGetPosition();
	InsExpSign sign = endSwitch == 0? INSPIRATION_SIGN : EXPIRATION_SIGN;

	finalPosition += ROTATIONAL_DIRECTION*sign*numSteps;
	return goToPositionAndFindSwitch(endSwitch, finalPosition, maxRawADCPressure);
}

bool goToPositionAndFindSwitch(int endSwitch, long finalPosition, float *maxRawADCPressure)
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
  motorMoveTo(finalPosition, sign, CONFIG_MOTOR_SPEED, CONFIG_MOTOR_SPEED);
#else
  unsigned char pwmPrescaler;
  unsigned int targetFreqSpeed = findFreqSpeed(CONFIG_MOTOR_SPEED, &pwmPrescaler);

  //Almost no acceleration
  motorPWMMoveTo(finalPosition, sign, targetFreqSpeed, targetFreqSpeed+1, pwmPrescaler);
  /*
  #if MOTOR_ACCELERATION == 0
 	unsigned long totalTime = 1000000 * (numSteps/STEPS_PER_REVOLUTION); //in microseconds
    motorPWMMoveTo(finalPosition, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE, totalTime, 0, pwmPrescaler);
  #else
    #ifndef ACCELERATE_EVERY_PULSE
 	  unsigned long totalTime = 1000000 * (numSteps/STEPS_PER_REVOLUTION); //in microseconds
      motorPWMMoveTo(finalPosition, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE,totalTime, totalTime/MOTOR_ACCELERATION, pwmPrescaler);
    #else
      motorPWMMoveTo(finalPosition, sign, targetFreqSpeed, targetFreqSpeed+MAX_RANGE_PWM_VALUE, pwmPrescaler);
    #endif
  #endif*/
#endif

  //long calls = 0;
  long lastDistanceToGo = motorDistanceToGo(), currentDistanceToGo;
  float currentPressure;
  while(motorDistanceToGo() != 0)
  {
	  currentDistanceToGo = motorDistanceToGo();
	  //we should update the watchdog only if the distance to go is changing.
	  if(lastDistanceToGo != currentDistanceToGo)
	  {
		lastDistanceToGo = currentDistanceToGo;
	  	watchdog_reset();
	  }

#ifdef USE_PRESSURE_SENSOR
	if(maxRawADCPressure != NULL)
	{
		//read the sample
		currentPressure = timer1_pSensor_raw_start_value(16);
		//currentPressure = 10;
		if(*maxRawADCPressure < currentPressure)
			*maxRawADCPressure = currentPressure;
	}
#endif
    // if we got to the limit switch
    if(digitalRead(pin) == 0)
    {/*
	#if DEBUG_CODE == 1
		txSerial.println(motorGetPosition());
	#endif*/
      motorStop();
      return true;
    }

    motorRun();
    //calls++;
  }
  motorStop();
  return false;
}


void goToEndLimitSwitchBlocking()
{
	//go to the end limit switch (expiration)
	if(moveStepsAndFindSwitch(1, MAXIMUM_START_SEQ_NUM_STEPS, NULL) == false)
	{
	  //something went wrong. Stopping the proccess
	  printLastMessage(MSG_NO_END_LIMIT_SWITCH, false);
	}
	//we set the current position to that which represents the end limit swicht position
	motorSetPosition(gActiveRespCfg.endLimitSwitchPos);
}

State verifyRestartMethod()
{
	State returnState = NA;

    switch(gActiveRespCfg.restartMethod)
    {
		case RESTART_WATCHDOG:
			//calculate the respiration parameters
			calculateMotorParam(true); //force update
			//set current screen as default screen
			setMenuState(DEFAULT_SCREEN);
			//activate the watchdog alarm
			setAlarm(ALARM_G);
			//we assume that it is at expiration phase,
			//and it will find the end limit switch and correct the motor position
			returnState = S4;
		break;
		case RESTART_UNEXPECTED:
			//turn on the beep
			timer1_beep_start();
			//shows the message indicating the problem
			printMessageLinesWaitResp("67008", 5);
			//turn off the beep
			timer1_beep_stop();
		break;
		default:
			//nothing to do here.
			break;
    }

	//update the restartMethod flag, so that it indicates that the system
	//dint restart properly if the flag isnt update again before the next shutdown
	gActiveRespCfg.restartMethod = RESTART_UNEXPECTED;
	//save the updated configuration
	saveRespConfiguration(&gActiveRespCfg);
	return returnState;
}

//Initial/Hidden configuration state
// Adjust the equipment.
// Block all others functions.
// S0 deals with start screen, hidden configuration and init. mode selection
State funcS0()
{
	CmdIhm checkVal;
	State returnState;
    //Load configuration
    loadRespConfiguration(&gActiveRespCfg);
#ifdef USE_PRESSURE_SENSOR
	//ensure that the configured calibration value is set
	set_calib_value(gActiveRespCfg.sensorCalibC1, gActiveRespCfg.sensorCalibC2);
#endif
	//Print any requirements related to the DEBUG_CODE define
#if DEBUG_CODE == 8
          txSerial.println(F("PIP PEEP"));
#endif


	//is the user pressing the Cfg button?
	if(ihm_get_key() == KEY_STAND_BY)
	{

		//Shows the screen saying that the calibration will be performed
		while(printMessageLinesWaitResp("?0008", 5) != CMD_OK); //OK to continue
		performStartProcedure(true, HIDDEN_CONFIG_NUM_TESTS);
		//Shows the screen saying that the calibration is done
		printMessageLinesWaitResp("@0008", 5); //any key to continue

		//delay any further button press.
		delayButtonCheck(1000);

		//The pressure sensor calibration can only be done without debug mode
#if defined(USE_PRESSURE_SENSOR) && DEBUG_CODE == 0
		char pressText[22];
		//variables used to configure the pressure sensor
		float tmpVal, x1 = 0, y1 = 0, x2 = 0, y2 = 60; //x2 = maxRawADCPressure
		bool leavePresCalib = true, firstIteration = true;
		//Determine the amount of pulses used in the calibration
		//Note that we do not have yet a posExp and posInsp
		//int pulses = (int)(PRESSURE_CALIBRATION_VOLUME_PERC*ROTATIONAL_DIRECTION*(gMotorParameters.cycles[NORMAL_CYCLE].stepPosExp-gMotorParameters.cycles[NORMAL_CYCLE].stepPosIns));
		int pulses = (int)(PRESSURE_CALIBRATION_VOLUME_PERC*ROTATIONAL_DIRECTION*gActiveRespCfg.endLimitSwitchPos); //we do not have yet a posExp and posInsp

		//shows the screen saying that it will calibrate the pressure sensor
		printMessageLinesWaitResp("TUV08", 5); //any key to continue
		//Calibrating message
		printMessageLine(MSG_CALIBRATING_ATM);

		// Obtain the current atmosphere pressure raw value.
		// Calibration should be done before sampling start
		// obtain the ADC value
		x1 = timer1_pSensor_raw_start_value(128);
		//y1 is always 0

		//dtostrf(x1, 4, 2, pressText);
		//printMessageLinesWaitResp("80", 2,pressText);

		//Explains the next test
		printMessageLinesWaitResp("TUQ08", 5); //any key to continue
		//x1 and y1 are already set.
		do
		{
			//leaveLoop will stays true if there is no change in the configuration
			leavePresCalib = true;

			//calculate sensorCalibC1 and sensorCalibC2 for the next iteration.
			//note that this must be performed BEFORE we obtain the value for the current iteration
			//since the value x2 of this iteration will serve to calibrate the next iteration if needed.
			gActiveRespCfg.sensorCalibC1 = CALC_SENSOR_C1(x1,y1,x2,y2);
			gActiveRespCfg.sensorCalibC2 = CALC_SENSOR_C2(gActiveRespCfg.sensorCalibC1,x1);

			//reset the x2 value
			x2 = 0;

			printMessageLine(MSG_PERFORMING_TESTS);
			//we must reach the indicated volume
			moveStepsAndFindSwitch(0, pulses, &x2); //x2 = maxRawADCPressure
			//delay 50ms
			for(int i = 0; i < 4; i++)
			{
				//make one last read
				if((tmpVal = timer1_pSensor_raw_start_value(16)) > x2)
					x2 = tmpVal;
			}

			//dtostrf(x2, 4, 2, pressText);
			//printMessageLinesWaitResp("80", 2,pressText);

			//C1 and C2 must be calculated here only if it is the first iteration, so that x2 has a valid value.
			if(firstIteration)
			{
				firstIteration = false;
				//calculate C1 and C2 based on x1, y1, x2, y2
				gActiveRespCfg.sensorCalibC1 = CALC_SENSOR_C1(x1,y1,x2,y2);
				gActiveRespCfg.sensorCalibC2 = CALC_SENSOR_C2(gActiveRespCfg.sensorCalibC1,x1);
			}

			//calculate the pressure using C1 and C2
			//this value is saved at y2
			y2 = ROUND_DOT5_FLOAT(ADCVALUETOCMH2O(x2, gActiveRespCfg.sensorCalibC1, gActiveRespCfg.sensorCalibC2));

			while(true)
			{
				//obtain the string
				dtostrf(y2, 3, 1, pressText);

				//Asks the user if the pressure is correct
				checkVal = printMessageLinesWaitResp("S00R8", 5,pressText);
				//if the user changes the pressure
				if(checkVal == CMD_UP)
				{
					y2 += 0.5f;
					leavePresCalib = false;
				}
				else if(checkVal == CMD_DOWN)
				{
					y2 -= 0.5f;
					leavePresCalib = false;
				} else if(checkVal == CMD_OK)
				{
					break;
				}
			}

			//secondly, we must reach the expiration switch
			moveStepsAndFindSwitch(1, pulses, NULL);
			//delay 100ms
			delay(100);
			//leaveLoop will be true if there was no change in the configuration
		} while(!leavePresCalib);

		//ensure that the configured calibration value is set
		set_calib_value(gActiveRespCfg.sensorCalibC1, gActiveRespCfg.sensorCalibC2);
#endif
		//we must save the current configuration, since it
		//contains information about the end limit switch position and pressure calibration
		saveRespConfiguration(&gActiveRespCfg);

#ifdef USE_PRESSURE_SENSOR
		// Sampling start should be called after the calibration is done
		timer1_pSensor_sampling_start();
#endif
		//delay any further button press.
		delayButtonCheck(1000);

		//move to threshold config screen/default screen
		return S2;
	} else {
#ifdef USE_PRESSURE_SENSOR
		// Sampling start should be called after the calibration is done
		timer1_pSensor_sampling_start();
#endif
	}

	//The restart flag must be checked after the pressure_sensor calibration procedure
	//in order to guarantee the start sampling call.
    //if the last reset was caused by a watchdog reset.
	returnState = verifyRestartMethod();
	//if we should change or current state due to the restart method
    if(returnState != NA)
		return returnState;


#if DEBUG_CODE != 0
	//Shows the start screen Debug Mode
	printMessageLine(DEBUG_MSG);
	delay(2000); //3s delay
#endif
	//Shows the start screen
	printMessageLines("0NP", 3);
	delay(3000); //3s delay

	//now, we must allow the user to select between 2 options:
	//Normal functions or tests
	//if there is an active fault, do not allow the user to skip the tests
	if(gActiveRespCfg.faultDetected)
		checkVal = printMessageLinesWaitResp("<000>", 5);
	else // if not, allow both options
		checkVal = printMessageLinesWaitResp("<00=>", 5);
	//if the user pressed Ok and there is no fault detected, open and go to S2 (stopped stage)
	if(!gActiveRespCfg.faultDetected && checkVal == CMD_OK)
	{
		//find end limit screen and go to pause state
		goToEndLimitSwitchBlocking();
		//move to threshold config screen/default screen
		return S2;
	}

	//otherwise, we move to S1 state
	return S1;
}
