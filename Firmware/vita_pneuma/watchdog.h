#ifndef WATCHDOG_H_
#define WATCHDOG_H_


#include <avr/wdt.h>
#include "Arduino.h"
#include "global.h"

#define watchdog_reset() wdt_reset()
bool getWatchdogStatus();
void watchdog_init(RespiratorInfo *respInfo);

#endif // WATCHDOG_H_
