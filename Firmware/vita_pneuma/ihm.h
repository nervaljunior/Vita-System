#ifndef IHM_H_
#define IHM_H_

#include "parameters.h"

// that gives you access to the standard types and constants of the Arduino
// language (this is automatically added to normal sketches, but not to
// libraries)
#include "Arduino.h"
#include "global.h"


#define KEY_ALR_PIN 8
#define KEY_CFG_PIN 7
#define KEY_UP_PIN 6
#define KEY_OK_PIN 5
#define KEY_DOWN_PIN 4
#define KEY_STAND_BY_PIN A5//0

//define whatever or not we are using the LCD 128x64
#define LCD_128x64 //if it is not set, the serial is used to informe modifications related to the screen
#ifdef LCD_128x64
  #define LCD_128x64_CLK_PIN 13
  #define LCD_128x64_MOSI_PIN 11
  #define LCD_128x64_RESET_PIN A4 //used only if RESET_LCD_OFTEN is defined

  #define LCD_WIDTH 128
  #define LCD_HEIGHT 64
  #define FONT_TYPE u8g_font_6x13 //9h font
  #define FONT_HEIGHT 9 //9h font
  #define FONT_HEIGHT_P (FONT_HEIGHT+3) //Additional space between lines
  #define FONT_HEIGHT_P_M (FONT_HEIGHT+1) //Additional space between lines
  #define FONT_WIDTH 6   //
  //#define FONT_TYPE u8g_font_8x13Br //10h font
  //#define FONT_HEIGHT 10 //10h font
  //#define FONT_HEIGHT_P (FONT_HEIGHT+6) //Additional space between lines
  //#define FONT_WIDTH 8   //

  #define GRAPH_UPDATE_MS 500
  #define LENGTH_GRAPH_PRESSURE_BUFFER 128
  #define HEIGHT_GRAPH_PRESSURE_BUFFER 44
  #define MAX_GRAPH_PRESSURE_VALUE 40
  #define MIN_GRAPH_PRESSURE_VALUE (-3)
  #define LINE_MEM_WIDTH (LENGTH_GRAPH_PRESSURE_BUFFER/8)
  #define GRAPH_SENSOR_FREQ (25.6f)

#ifdef RESET_LCD_OFTEN
 void resetLCD();
#endif

#else
/* //if the 128x64 isn't used, I2C pins should be used for the 20x4 LCD.
  #define LCD_RS_PIN 8
  #define LCD_EN_PIN 9
  #define LCD_D4_PIN 4
  #define LCD_D5_PIN 5
  #define LCD_D6_PIN 6
  #define LCD_D7_PIN 7
*/
#endif


// potentiometers pins
#define POT_VOLUME A1
#define POT_BPM A2 //= frequency
#define POT_IE_RATIO A3

typedef enum keys{
	KEY_NOTPRESSED  = 0,
	KEY_STAND_BY          = 1,
	KEY_DOWN          = 2,
	KEY_OK        = 3,
	KEY_UP        = 4,
	KEY_CFG         = 5,
	KEY_ALR         = 6
} Key;



typedef struct potsinfo{
  int potbpm;
  int potieratio;
  int potvolume;
} PotsInfo;


typedef enum fsmstate{
  //MENUSELECIONEACONFIG,
  NO_SCREEN,
  DEFAULT_SCREEN,
//  BEEP_SCREEN,
//  POP_OFF_SCREEN,
#ifdef USE_PRESSURE_SENSOR
//  CALI_SCREEN,
  CONFIG_SCREEN,
  GRAPH_SCREEN
#endif
} StateIHM;

#ifdef USE_PRESSURE_SENSOR

enum {
  NOTHING_SELECTED,
  PIP_SELECTED,
  PEEP_SELECTED,
  TRIGGER_SELECTED,
  MD_SELECTED
};

#endif

enum {
  //MENUSELECIONEACONFIG,
  VOL_CHANGED = 1,
  BPM_CHANGED = 2,
  IE_CHANGED = 4
};

typedef enum cmd{
	CFGA = 0,
	CFGB = 1,
	CFGC = 2,
	CFGD = 3,
	CFGE = 4,
	CFGF = 5,
  CONFIG,
  NEWDATA,
  NONE,
  CMD_STAND_BY,
  CMD_DOWN,
  CMD_OK,
  CMD_UP,
  CMD_ALR,
} CmdIhm;

typedef enum MessageID {
	MSG_NO_MSG = '0',
	MSG_NO_END_LIMIT_SWITCH = '1',
	MSG_NO_START_LIMIT_SWITCH = '2',
	MSG_STEP_LOSS_ERROR = '3',
	MSG_START_IN_3S = '4',
	MSG_CALIBRATING_ATM = '5',
	MSG_WRONG_RESTART_0 = '6',
	MSG_WRONG_RESTART_1 = '7',
	MSG_OK_TO_CONTINE = '8',
	MSG_MOVING_TEST_0 = '9',

	MSG_CANCEL = ':',
	MSG_SYSTEM_OFF = ';',
	MSG_INIT_MODE = '<',
	MSG_OK_DEFAULT = '=',
	MSG_CFG_TEST = '>',
	MSG_VOL_CAL = '?',
	MSG_CALI_CONC = '@',

	MSG_MOVING_TEST_1 = 'A',
	MSG_MOVING_TEST_2 = 'B',
	MSG_MOVING_TEST_3 = 'C',
	MSG_PERFORMING_TESTS = 'D',
	MSG_TEST_CONCLUDED = 'E',
	MSG_TEST_SUCCESS = 'F',
	MSG_TEST_FAIL = 'G',
	MSG_CFG_TO_REPEATE = 'H',
	MSG_ALARM_TEST_0 = 'I',
	MSG_ALARM_TEST_1 = 'J',
	MSG_ALARM_TEST_2 = 'K',
	MSG_OK_LISTEN = 'L',
	MSG_ALR_NOT_LISTEN = 'M',

	INTRO_MSG_1 = 'N',

	MSG_ALR_SYS_ERROR = 'O',

	INTRO_MSG_2 = 'P',
	MSG_OF_POP_OFF = 'Q',
	MSG_ADJUST_IF_NEEDED = 'R',
	MSG_OBTAINED_PRESSURE = 'S',

	MSG_CAL_PRESS_0 = 'T',
	MSG_CAL_PRESS_1 = 'U',
	MSG_CAL_PRESS_2 = 'V',
	MSG_SYS_FAIL = 'W',
	MSG_SHUT_DOWN = 'X',
	MSG_OK = 'Y',
	MSG_PAUSED_SYSTEM
#if DEBUG_CODE != 0 //Debug  must be the last message
	, DEBUG_MSG
#endif
} MessageID;

/*
#if STEP_POP_OFF_TEST_VOLUME > 0
	MSG_OK_POP_OFF = 'N',
#endif
#if STEP_POP_OFF_TEST_VOLUME > 0
	MSG_POP_OFF_TEST_0 = 'P',
	MSG_POP_OFF_TEST_1 = 'Q',
	MSG_POP_OFF_TEST_2 = 'R',
	MSG_POP_OFF_TEST_3 = 'S',
#endif
*/


struct RespiratorInfo;
struct RespiratorEstInfo;
void ihm_init(RespiratorInfo* respinfoptr, RespiratorEstInfo *respestptr);

CmdIhm  ihm_newCmd(Key key,PotsInfo pot);

CmdIhm ihm_check();

PotsInfo ihm_checkPots();

void requestScreenUpdate();
void printLastMessage(char msgID, bool beep);
void printMessageLine(char msg);
void printMessageLines(const char* msgs, unsigned char len, char* secondLine = NULL);
void popoffInfoScreen(float pressure);
CmdIhm printMessageLinesWaitResp(const char* msgs, unsigned char len, char* secondLine = NULL);
void delayButtonCheck(long time);

Key ihm_get_key();

RespiratorInfo* ihm_getInfo();
void setMenuState(StateIHM state);
StateIHM getMenuState();



#endif /* IHM_H_ */
