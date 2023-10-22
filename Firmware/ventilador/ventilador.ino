#include "global.h"
#include "motor.h"
#include "ihm.h"
#include "pwm.h"
#include "beep.h"


State gState;
RespiratorInfo gActiveRespCfg;
MotorParameters gMotorParameters;
StartConfig gStartConfig;
unsigned long gInitialBreathTime;
#ifdef DEBUG_CODE
  static unsigned long totalBreathTime;
  static unsigned long inspTime, initExpTime, expTime=0;
  static int contResp = 0;// 
#endif

void setup() {

  motorInit();
  
  #ifdef DEBUG_CODE
    //We can use serial for debugging
    Serial.begin(9600); 
    Serial.println("Starting...");
  #endif

  ihm_init(&gActiveRespCfg);
  beep_init();
  //  
  pinMode(LIMIT_SWITCH_INI_PIN, INPUT_PULLUP);      // 
  pinMode(LIMIT_SWITCH_END_PIN, INPUT_PULLUP);      // 
  gState = S0;
}


void loop() 
{ // 
  static State oldState;
  State newState;
  unsigned long now;

  switch(gState)
  {
    case S0: 
      newState = funcS0(oldState);
    break;
    case S1:
      newState = funcS1(oldState);
    break;
    case S2:
      newState = funcS2(oldState);
    break;
    case S3:     
      #ifdef DEBUG_CODE
        if(gInitialBreathTime > 0)
        {
          expTime = millis() - initExpTime;
          totalBreathTime = millis() - gInitialBreathTime;
          Serial.print("I:E: 1:");        
          Serial.print((double)expTime/(double)inspTime);     
          Serial.print(", RPM: ");        
          Serial.println(60/(totalBreathTime/(float)1000));     
        }
      #endif 
      //obtain the new initial breath time
      gInitialBreathTime = millis();
      newState = funcS3(oldState);
      #ifdef DEBUG_CODE
        initExpTime = millis();    
        inspTime = initExpTime - gInitialBreathTime;  
      #endif
    break;
    case S4:
      newState = funcS4(oldState);
    break;
  }
  oldState = gState;
  
  if(newState != gState)
    gState = newState;
}
void reportErr(char *erro)
{
  #ifdef DEBUG_CODE
    Serial.println(erro);
  #endif  
  beep_start();
  while(1) { 
	beep_update();
  }
}
