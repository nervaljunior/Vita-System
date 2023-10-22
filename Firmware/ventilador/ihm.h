#ifndef IHM_H_
#define IHM_H_

// that gives you access to the standard types and constants of the Arduino
// language (this is automatically added to normal sketches, but not to
// libraries)
#include "Arduino.h"


// potentiometers pins
const int pot1 = A1;
const int pot2 = A2;

typedef enum keys{
	KEY_NOTPRESSED  = 0,
	KEY_RT          = 1,
	KEY_UP          = 2,
	KEY_DOWN        = 3,
	KEY_LEFT        = 4,
	KEY_SEL         = 5
} Key;

// We use this struct to share info between the IHM
// and others part of the code
typedef struct info{
	int volume;
	int pos_exp;
  int bpm;
  float ie_ratio;
} RespiratorInfo;

typedef struct potsinfo{
  int potbpm;
  int potieratio;
} PotsInfo;


typedef enum fsmstate{
  MENUSELECIONEACONFIG,
  TELAPADRAO,
  MENUINPUTCHANGED,
  MENUKEYPRESSED,
} StateIHM;

typedef enum cmd{
	CFGA = 0,
	CFGB = 1,
	CFGC = 2,
	CFGD = 3,
	CFGE = 4,
	CFGF = 5,
  SELECT,
  NEWDATA,
  NONE,
  CMD_KEY_LEFT,
  CMD_KEY_RT,
  CMD_KEY_UP,
  CMD_KEY_DOWN
} CmdIhm;

void ihm_init(RespiratorInfo* respinfoptr);

CmdIhm  ihm_newCmd(Key key,PotsInfo pot);

CmdIhm ihm_check();

void ihm_initScreen();

PotsInfo ihm_checkPots();

Key ihm_get_key();

RespiratorInfo* ihm_getInfo();

extern StateIHM g_menu_state;
extern RespiratorInfo gActiveRespCfg;

#endif /* IHM_H_ */
