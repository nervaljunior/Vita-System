#include <avr/eeprom.h>
#include "Arduino.h"
#include "configmanager.h"
#include "alarm.h"
#include "global.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

//load de con
void loadRespConfiguration(RespiratorInfo *respiratorInfo)
{
  eeprom_read_block((void*)respiratorInfo, 0, sizeof(RespiratorInfo));
/*
#ifndef ALWAYS_MAX_EXP_POS
  //if the obtained parameters are invalid
  //we must adjust it
  if(respiratorInfo->pos_exp < MIN_POS_EXP || respiratorInfo->pos_exp > MAX_POS_EXP)
    respiratorInfo->pos_exp = DEFAULT_POS_EXP;
#else
  #if DEBUG_CODE != 0
    txSerial.println(F("Def. ExpPos."));
  #endif
  respiratorInfo->pos_exp = DEFAULT_POS_EXP;
#endif*/
	if(respiratorInfo->volume_pulses < MIN_VOLUME || respiratorInfo->volume_pulses > MAX_VOLUME)
		respiratorInfo->volume_pulses = DEFAULT_VOLUME;

	if(isnan(respiratorInfo->bpm) || respiratorInfo->bpm < MIN_BPM || respiratorInfo->bpm > MAX_BPM)
		respiratorInfo->bpm = DEFAULT_BPM;

	if(isnan(respiratorInfo->ie_ratio) || respiratorInfo->ie_ratio < MIN_IE_RATIO || respiratorInfo->ie_ratio > MAX_IE_RATIO)
		respiratorInfo->ie_ratio = DEFAULT_IE_RATIO;

	if(respiratorInfo->pip_max < MIN_PIP || respiratorInfo->pip_max > MAX_PIP)
		respiratorInfo->pip_max = DEFAULT_PIP;

	if(respiratorInfo->peep_min < MIN_PEEP_MIN || respiratorInfo->peep_min > MAX_PEEP_MIN)
		respiratorInfo->peep_min = DEFAULT_PEEP_MIN;

	if(isnan(respiratorInfo->trigger) || respiratorInfo->trigger < MIN_TRIGGER || respiratorInfo->trigger > MAX_TRIGGER)
		respiratorInfo->trigger = DEFAULT_TRIGGER;

	if(isnan(respiratorInfo->sensorCalibC1))
		respiratorInfo->sensorCalibC1 = DEFAULT_SENSOR_C1;

	if(isnan(respiratorInfo->sensorCalibC2))
		respiratorInfo->sensorCalibC2 = DEFAULT_SENSOR_C2;

	if(respiratorInfo->restartMethod < 0  || respiratorInfo->restartMethod >= TOTAL_RESTART_METHODS)
		respiratorInfo->restartMethod = DEFAULT_RESTART_METHOD;

	if(respiratorInfo->endLimitSwitchPos < -MAXIMUM_START_SEQ_NUM_STEPS || respiratorInfo->endLimitSwitchPos == 0 || respiratorInfo->endLimitSwitchPos > MAXIMUM_START_SEQ_NUM_STEPS)
		respiratorInfo->endLimitSwitchPos = DEFAULT_NUMBER_STEPS;

#ifdef USE_PRESSURE_SENSOR
  if(respiratorInfo->op_mode < 0 || respiratorInfo->op_mode >= TOTAL_MODES)
    respiratorInfo->op_mode = DEFAULT_OP_MODE;
#else
  //mandatory mode if there is no pressure sensor
  respiratorInfo->op_mode = N_MODE;
#endif

}

void saveRespConfiguration(RespiratorInfo *respiratorInfo)
{
  //N_MODE requires that the alarms be reseted
  if(respiratorInfo->op_mode == N_MODE)
    resetAllAlarms();

  eeprom_write_block((const void*)respiratorInfo, 0, sizeof(RespiratorInfo));
}
