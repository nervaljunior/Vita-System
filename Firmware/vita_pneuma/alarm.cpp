#include "Arduino.h"
#include "alarm.h"
#include "global.h"
#include "timer1.h"
#include "ihm.h"
#include "SendOnlySoftwareSerial.h"

const char alarmTexts[][13]  =   {{"OPERA NORMAL"}, //BE CAREFUL WITH THE MAXIMUM SIZE!
                                  {"ERR NO MOTOR"}, //ALARM A
                                  {"VOL Ñ ENTREG"}, //ALARM B
                                  {"QUED PRESSÃO"}, //ALARM C
                                  {"PEEP VOLÁTIL"}, //ALARM D
                                  {"PIP EXCEDIDA"}, //ALARM E
                                  {"PEEP MIN VIO"}, //ALARM F
                                  {"PROBLEMA TEC"}, //ALARM G
                                  {"SIST PAUSADO"},
                                  {"PRESSIONE OK"}};

static bool gAlarmRinging, gSilenceAlarm;
static unsigned long gLastSilenceTime;
static unsigned long currentCycleAlarms = 0;

void setAlarm(AlarmCode alarmCode)
{
  //we must verify if this is a new active alarm
  if(!(gActiveRespEst.alarms.activeAlarmsMask & (1 << (alarmCode-1))))
  {
    //Are we already ringing?
    if(!gAlarmRinging)
    {
      gAlarmRinging = true;
      timer1_beep_start();
    }
  }
  //Note that, is this is not a new alarm, we are silenced (waiting to start ringing again in SILENCE_MILLIS_TIME ms)
  //or ringing

  //make sure the alarm is set
  gActiveRespEst.alarms.activeAlarmsMask |= 1 << (alarmCode-1);
  //also update the current cycle alarm
  currentCycleAlarms |= 1 << (alarmCode-1);
}
void resetAllAlarms()
{
  if(gAlarmRinging)
  {
    timer1_beep_stop();
    gAlarmRinging  = false;
  }
  gActiveRespEst.alarms.alarmTextIndex = OP_NORMAL;
  gActiveRespEst.alarms.activeAlarmsMask = 0;
  gActiveRespEst.alarms.inactiveAlarmsMask = 0;
  currentCycleAlarms = 0;
  gLastSilenceTime = 0;
  gSilenceAlarm = false;
  requestScreenUpdate();
}
void alarmInit()
{
  //reset the alarm info
  resetAllAlarms();
}

void alarmButtonPress()
{
  //delay any further button press.
  delayButtonCheck(1000);
  if(gActiveRespEst.alarms.inactiveAlarmsMask != 0)
  {
    gActiveRespEst.alarms.inactiveAlarmsMask = 0;
    requestScreenUpdate();
  }
  //if the alarm is ringing
  if(gAlarmRinging)
  {
    //if there is an active alarm still present
    if(gActiveRespEst.alarms.activeAlarmsMask)
    {
       gSilenceAlarm = true;
       gLastSilenceTime = millis();
    }
    gAlarmRinging  = false;
    timer1_beep_stop();
  } else {
    //if the alarm was not ringing, but there are still alarms on
    if(gActiveRespEst.alarms.activeAlarmsMask)
    {
       gAlarmRinging  = true;
       timer1_beep_start();
       gSilenceAlarm = false;
    }
  }
}
void alarmUpdate()
{
  //We must save into inactive alarm all alarms that are not on anymore
  gActiveRespEst.alarms.inactiveAlarmsMask |= (~currentCycleAlarms & gActiveRespEst.alarms.activeAlarmsMask);
  //Then, we update the active alarms
  gActiveRespEst.alarms.activeAlarmsMask = currentCycleAlarms;
  //finally, reset the current cycle alarms
  currentCycleAlarms = 0;

  if(gActiveRespEst.alarms.activeAlarmsMask == 0 && gActiveRespEst.alarms.inactiveAlarmsMask == 0)
  {
    if(gAlarmRinging)
    {
      timer1_beep_stop();
      gAlarmRinging  = false;
    }
    if(gSilenceAlarm)
      gSilenceAlarm = false;

  } //If the user did silence the alarm, but the silence time has passed
  else if(gSilenceAlarm && (millis() - gLastSilenceTime > SILENCE_MILLIS_TIME))
  {
     gAlarmRinging  = true;
     timer1_beep_start();
     gSilenceAlarm = false;
  }
}

void updateAlarmText()
{
  static unsigned long lastAlarmUpdate = 0;
  //at least 1 seg. on screen
  if(millis() - lastAlarmUpdate < MIN_MILLIS_ALARM_TEXT)
    return;
  lastAlarmUpdate = millis();

  //If we are at the paused state
  if(getSystemState() == S2)
  {
	  if(gActiveRespEst.alarms.alarmTextIndex == ALARM_PAUSED)
	  	gActiveRespEst.alarms.alarmTextIndex = ALARM_PRES_OK;
	  else
	  	gActiveRespEst.alarms.alarmTextIndex = ALARM_PAUSED;
	  return;
  }


  //do we have any alarm at all?
  if(gActiveRespEst.alarms.activeAlarmsMask == 0 && gActiveRespEst.alarms.inactiveAlarmsMask == 0)
  {
    gActiveRespEst.alarms.alarmTextIndex = OP_NORMAL;
    return;
  }
  int currentAlarm = gActiveRespEst.alarms.alarmTextIndex;
  //cicle through all alarms
  for(int i = 1; i < ALARMTEXTSIZE; i++)
  {
    currentAlarm++;
    if(currentAlarm >= ALARMTEXTSIZE)
      currentAlarm = 1;

    if((gActiveRespEst.alarms.activeAlarmsMask & (1 << (currentAlarm-1))) ||
       (gActiveRespEst.alarms.inactiveAlarmsMask & (1 << (currentAlarm-1))))
    {
      gActiveRespEst.alarms.alarmTextIndex = currentAlarm;
      return;
    }
  }
  //should not happen
  #if DEBUG_CODE != 0
    txSerial.println(F("BUG"));
  #endif
  gActiveRespEst.alarms.alarmTextIndex = OP_NORMAL;
}
