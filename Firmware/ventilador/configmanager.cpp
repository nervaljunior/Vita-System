#include <avr/eeprom.h>
#include "configmanager.h"
#include "global.h"

//load de con
void loadRespConfiguration(RespiratorInfo *respiratorInfo, int position)
{
  eeprom_read_block((void*)respiratorInfo, (void*)(position*sizeof(RespiratorInfo)), sizeof(RespiratorInfo));
  //if the obtained parameters are invalid
  //we must adjust it
  if(isnan(respiratorInfo->volume) || respiratorInfo->pos_exp < POSMIN || respiratorInfo->pos_exp > POSMAX)
    respiratorInfo->pos_exp = DEFAULT_POS_EXP;
    
  if(respiratorInfo->volume < 1 || respiratorInfo->volume > 100)
    respiratorInfo->volume = DEFAULT_VOLUME;
    
  //TODO: hardcoded limit BPM values
  if(isnan(respiratorInfo->bpm) || respiratorInfo->bpm < 10 || respiratorInfo->bpm > 30)
    respiratorInfo->bpm = DEFAULT_BPM;

  //TODO: hardcoded limit BPM IE ratio values
  if(isnan(respiratorInfo->ie_ratio) || respiratorInfo->ie_ratio < 1 || respiratorInfo->ie_ratio > 3)
    respiratorInfo->ie_ratio = DEFAULT_IE_RATIO;
}

void saveRespConfiguration(RespiratorInfo *respiratorInfo, int position)
{
  eeprom_write_block((const void*)respiratorInfo, (void*)(position*sizeof(RespiratorInfo)), sizeof(RespiratorInfo));
}
