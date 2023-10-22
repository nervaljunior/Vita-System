#include "alarm.h"
#include "global.h"
#include "pressure.h"
#include "motor.h"
#include "ihm.h"
#include "pwm.h"
#include "timer1.h"
#include "configmanager.h"
#include "watchdog.h"

unsigned long gInitialBreathTime;
static unsigned long inspTime, initExpTime, expTime=0;
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
  SendOnlySoftwareSerial txSerial(1);  // Tx pin
#endif

static State gSystemState;
RespiratorInfo gActiveRespCfg;
RespiratorEstInfo gActiveRespEst;
MotorParameters gMotorParameters;
State getSystemState()
{
	return gSystemState;
}

void setup() {
	analogReference(INTERNAL);

	timer1_init();

	motorInit();

	#if DEBUG_CODE != 0
	//We can use serial for debugging
	txSerial.begin(115200); 
	#endif

	ihm_init(&gActiveRespCfg, &gActiveRespEst);
	alarmInit();

	//  
	pinMode(LIMIT_SWITCH_INI_PIN, INPUT_PULLUP);      // 
	pinMode(LIMIT_SWITCH_END_PIN, INPUT_PULLUP);      // 
	gSystemState = S0;


	//watchdog is start at S3 or S4 states.
	//Set the current cycle (used for motor tests)
	gActiveRespEst.currentCycle = CYCLES_PER_MOTOR_TEST;
}

void cycleUpdate()
{
  unsigned long totalBreathTime;
  unsigned long now;
  now = millis();
  expTime = now - initExpTime;
  totalBreathTime = now - gInitialBreathTime;   
  //every start breath cicle we update the alarm
  alarmUpdate(); 
  
  if(inspTime != 0)
  {
    gActiveRespEst.ie_ratio_mea = ((float)expTime/(float)inspTime); //0.05f +  // 0.05f for round purposes
    gActiveRespEst.bpm_mea = (60/(totalBreathTime/(float)1000));
    //Volume is calculated in end of inspiration phase
    requestScreenUpdate();
  }
  //obtain the new initial breath time
  gInitialBreathTime = millis();
}
void loop() 
{ // 
	static State oldState;
	State newState = gSystemState;

	switch(gSystemState)
	{
	case S0: 
		//Initial/Hidden configuration state
		newState = funcS0();
	break;
	case S1:
		//Test state
		newState = funcS1();
	break;
	case S2:
		//Stopped state
		newState = funcS2();
		inspTime = 0; //avoid wrong I-E calculation.
	break;
	case S3: 
		//Inspiration state
		cycleUpdate();
		newState = funcS3(oldState);
	break;
	case S4:
		initExpTime = millis();    
		inspTime = initExpTime - gInitialBreathTime;  
		//Expiration state
		newState = funcS4(oldState);
	break;
	default:
		//Blocking/beeping error message.
		printLastMessage(MSG_SYS_FAIL, true);
		
	}
	oldState = gSystemState;

	if(newState != gSystemState)
		gSystemState = newState;
}
