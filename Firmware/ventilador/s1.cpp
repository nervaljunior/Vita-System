#include "global.h"
#include "configmanager.h"
#include "beep.h"

// Shows the selection of the actual configuration
State funcS1(State oldState)
{
  CmdIhm checkVal = NONE;
	unsigned long timeref;
	
	// Wait Confirmation form IHM
	timeref = millis();
	while(	checkVal != CFGA && checkVal != CFGB && checkVal != CFGC &&
			checkVal != CFGD && checkVal != CFGE && checkVal != CFGF){
				
		checkVal = ihm_check();
		
		// Update timer if the user has pressed some key
		if(	checkVal==  CMD_KEY_LEFT 	|| checkVal==CMD_KEY_RT || 
			checkVal==CMD_KEY_UP 		|| checkVal==CMD_KEY_DOWN){
			timeref = millis();
		}
		
		/**
		* If beep is not running and the timer counting has reached its limit,
		* then start the beep.
		* Once the beep is running,  beep_update performs some "animations" on
		* the beep.
		*/
		if(!beep_isRunning()){
			if((millis()-timeref)>TIMETOBEEPINTHECFGSCREEN_MS){
				beep_start();
			}
		}else{
			beep_update();
		}
	}
	
	// beep is stopped after user has pressed some button
	beep_stop();
	
  //get the index of the based on the first config
  gStartConfig.activeConfigPos = checkVal;

  //load the configuration based on the position
  loadRespConfiguration(&gActiveRespCfg, gStartConfig.activeConfigPos);


  #ifdef DEBUG_CODE
    Serial.print("loaded configuration: ");
    Serial.println(gStartConfig.activeConfigPos);
    Serial.println("volume, pos_exp, bpm, ie_ratio:");
    Serial.println(gActiveRespCfg.volume);
    Serial.println(gActiveRespCfg.pos_exp);
    Serial.println(gActiveRespCfg.bpm);
    Serial.println(gActiveRespCfg.ie_ratio);
    Serial.println("Motor Stopped.");
  #endif

  //From state S1 to state S2, the motor must be stopped.
  //This is already the case.
  return S2;
}
