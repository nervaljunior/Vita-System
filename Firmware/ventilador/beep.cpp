#include "beep.h"
#include "TimerOne.h"

// PIN B4 is the pin 12 of the arduino
// These 3 macros should correspond to the same port and pin
#define BUZZER_ARDUINO_PIN 12
// This macro should be PINA, PINB, PINC, PIND, ...
#define BUZZER_MCU_PORTPIN PINB
// This macro should be one of the values 0,1,2,3,4,5,6,7
#define BUZZER_MCU_PIN 4


static bool gBeepIsRunnig = false;


static void callback(){
	// writing 1 in PINB3 toogles the state of the pin. 
	// See Section "14.2.2 Toggling the Pin" of the ATmega328 datasheet
	//sbi(BUZZER_MCU_PORTPIN, BUZZER_PIN);
	BUZZER_MCU_PORTPIN |= _BV(BUZZER_MCU_PIN);
}

void beep_init(){
	pinMode(BUZZER_ARDUINO_PIN, OUTPUT);
	Timer1.initialize(1000);         // initialize timer1, and set a 1ms period
}

void beep_start(){
	Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
	gBeepIsRunnig = true;
}

void beep_stop(){
	// attaches callback() as a timer overflow interrupt
	Timer1.detachInterrupt();
	// Clear BUZZER PIN
	digitalWrite(BUZZER_ARDUINO_PIN,LOW);
	gBeepIsRunnig = false;
}

bool beep_isRunning(){
	return gBeepIsRunnig;
}

void beep_update(){
	static unsigned long time_new,time_old=0;;
	enum BeepState{ON,OFF} ;
	const unsigned short beepTimeMs = 200;
	static enum BeepState beep_state = OFF;

	if(gBeepIsRunnig){ // we need update beep only if it is running
		time_new = millis();

		//wait, at least, the time for a single beep
		if((time_new-time_old)>beepTimeMs){
			
			// update TCCR2A register to enable output
			
			if(beep_state == OFF){
				// Update Frequency in the register or turn on buzzer
				Timer1.attachInterrupt(callback);
				// now the beep is ON
				beep_state = ON; 
				
				// TODO: Also change frequency? This would need more code
				//	and more tests
			}
			else{
				// Update Frequency in the register or turn off buzzer
				Timer1.detachInterrupt();
				// Clear BUZZER PIN
				digitalWrite(BUZZER_ARDUINO_PIN,LOW);
				
				// TODO: Also change frequency? This would need more code
				//	and more tests
				beep_state = OFF;
			}
			// update time
			time_old=time_new;
		}
	}
}