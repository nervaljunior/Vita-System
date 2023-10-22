#ifndef ALARM_H
#define ALARM_H

#include "parameters.h"

#define MIN_MILLIS_ALARM_TEXT 1000
#define SILENCE_MILLIS_TIME 1200000 //20min

typedef enum {
  OP_NORMAL = 0,
  ALARM_A = 1,
  ALARM_B = 2,
  ALARM_C = 3,
  ALARM_D = 4, //PEEP is not stabilized
  ALARM_E = 5,
  ALARM_F = 6,
  ALARM_G = 7,
  ALARMTEXTSIZE,
  ALARM_PAUSED = ALARMTEXTSIZE,
  ALARM_PRES_OK
} AlarmCode;


typedef struct AlarmInfo
{
  unsigned long alarmBtPressTime;
  unsigned long activeAlarmsMask;
  unsigned long inactiveAlarmsMask;
  char alarmTextIndex;
} AlarmInfo;


extern const char alarmTexts[][13];

void alarmInit();
void updateAlarmText();
void setAlarm(AlarmCode alarmCode);
void alarmButtonPress();
void alarmUpdate();
void resetAllAlarms();

#endif
