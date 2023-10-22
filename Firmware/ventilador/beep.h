#ifndef BEEP_H_
#define BEEP_H_
#include "Arduino.h"

void beep_init();

void beep_start();

void beep_stop();

bool beep_isRunning();

void beep_update();

#endif // BEEP_H_
