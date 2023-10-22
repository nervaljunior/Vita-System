#include "watchdog.h"
#include "configmanager.h"
#include <avr/eeprom.h>
#include "watchdog.h"

static RespiratorInfo *g_respInfo;
static bool watchdogStarted = false;
// ISR that is called on Watchdog Time-out
ISR(WDT_vect){
	g_respInfo->restartMethod = RESTART_WATCHDOG;
	saveRespConfiguration(g_respInfo);
}
bool getWatchdogStatus()
{
	return watchdogStarted;
}

void watchdog_init(RespiratorInfo *respInfo){
	cli();
	watchdog_reset();
	watchdogStarted = true;

	g_respInfo = respInfo;

	// init WDT that resets every X seconds (e.g., WDP(3:0)=0110 fires every 1s)
	// Time-out options for ATmega328P
	// WDP(3:0)=0000 -> 16ms
	// WDP(3:0)=0001 -> 32ms
	// WDP(3:0)=0010 -> 64ms
	// WDP(3:0)=0011 -> 125ms
	// WDP(3:0)=0100 -> 250ms
	// WDP(3:0)=0101 -> 500ms
	// WDP(3:0)=0110 -> 1000ms
	// WDP(3:0)=0111 -> 2000ms
	// WDP(3:0)=1000 -> 4000ms
	// WDP(3:0)=1001 -> 8000ms

	//WARNING: if SET_EXPIRATION_SPEED is changed, one might have to increase the watchdog reset time since breath and plateau procedures do not call watchdogreset.

	// Enter Watchdog Configuration mode
	// (this is necessary before changing the WDTCSR register):
	WDTCSR |= (1<<WDCE) | (1<<WDE);

	// Set Watchdog settings:
	//WDTCSR = (1<<WDIE) | (1<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0); //1000ms
	WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0); //8000ms

	sei();
	//return ret;
}