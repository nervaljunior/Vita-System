#include "alarm.h"
#include "global.h"
#include "configmanager.h"
#include "timer1.h"
#include "ihm.h"
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif


/*
#if STEP_POP_OFF_TEST_VOLUME > 0

void performPopoffProcedure()
{
	float maxRawADCPressure = 0; //passed to the function
	//we configure the system to run at most the number of steps

	//firstly, we must reach the indicated volume
	moveStepsAndFindSwitch(0, STEP_POP_OFF_TEST_VOLUME, &maxRawADCPressure);
	//prints the pop off screen with the acquired pressure
	popoffInfoScreen(maxRawADCPressure);
	delay(2000);
	//secondly, we must reach the expiration switch
	moveStepsAndFindSwitch(1, STEP_POP_OFF_TEST_VOLUME, &maxRawADCPressure);
}
#endif
*/

//Test state
State funcS1()
{
	CmdIhm checkVal = NONE;
	//unsigned long timeref;

	//delay any further button press.
	delayButtonCheck(1000);

	//Show the information about the motor move test.
	printMessageLinesWaitResp("9ABC8", 5);  //any key to continue
	//Show the testing message
	printMessageLine(MSG_PERFORMING_TESTS);
	//Perform the tests
	performStartProcedure(false, 2);
	//Show the successful screen
	printMessageLinesWaitResp("EF008", 5); //any key to continue

	//delay any further button press.
	delayButtonCheck(1000);

	//The next step is to keep playing the beep
	printMessageLinesWaitResp("IJK08", 5); //any key to continue
	//Start the beep
	timer1_beep_start();
	do
	{
		//asks if the user hears it
		checkVal = printMessageLinesWaitResp("E00LM", 5); //any key to continue
		if(checkVal == CMD_ALR) //if the user decided to cancel
		{
			timer1_beep_stop();
			//This call will stop the process
			systemFail(MSG_SYS_FAIL);
		}
	} while(checkVal !=  CMD_OK);
	timer1_beep_stop();

	//delay any further button press.
	delayButtonCheck(1000);

/*
#if STEP_POP_OFF_TEST_VOLUME > 0
	//Explains the pop-off test
	printMessageLinesWaitResp("PQRS8", 5); //any key to continue
	printMessageLine(MSG_PERFORMING_TESTS);
	do
	{
		performPopoffProcedure();
		delay(2000);
		//Asks the user if everything went fine
		checkVal = printMessageLinesWaitResp("E0NHO", 5); //any key to continue

		if(checkVal == CMD_ALR) //if the user decided to cancel
		{
			//This call will stop the process
			systemFail(MSG_SYS_FAIL);
		}
	} while(checkVal !=  CMD_OK);
#endif
*/


	//if it gets to here, it means everything went fine.
	//so we must update the fault flag
	gActiveRespCfg.faultDetected = false;
	//save the updated configuration
	saveRespConfiguration(&gActiveRespCfg);

	//From state S1 to state S2, the motor must be stopped.
	//This is already the case.
	return S2;
}
