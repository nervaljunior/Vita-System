#include "ihm.h"
#include "alarm.h"
#include "global.h"
#include "timer1.h"
#include <Stream.h>
#include "watchdog.h"
#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif



#ifndef LCD_128x64
  //if the 128x64 isn't used, I2C pins should be used for the 20x4 LCD.
/*
  #if STEP_PIN == LCD_EN_PIN
    #error "While using the LCD, make sure STEP_PIN differs from LCD_EN_PIN."
  #endif
  // include the library code for LCD 16x02
  #include <LiquidCrystal.h>
  // initialize the library by associating any needed LCD interface pin
  // with the arduino pin number it is connected to
  LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);
  */
#else
  #include <U8glib.h>
  //U8GLIB_ST7920_128X64 lcd_128x64(LCD_128x64_CLK_PIN, LCD_128x64_MOSI_PIN, U8G_PIN_NONE, U8G_PIN_NONE);
  U8GLIB_ST7920_128X64 lcd_128x64(LCD_128x64_CLK_PIN, LCD_128x64_MOSI_PIN, U8G_PIN_NONE, U8G_PIN_NONE, U8G_PIN_NONE);//LCD_128x64_RESET_PIN);

  char gPrintPsensor[LENGTH_GRAPH_PRESSURE_BUFFER];
  static int gBufferPrintIdx;

#endif


#ifdef USE_PRESSURE_SENSOR
	#define INITIAL_SCREEN CONFIG_SCREEN
#else
	#define INITIAL_SCREEN DEFAULT_SCREEN
#endif

static char updatingScreen;
static unsigned long last_time_button_check =0;

#define PERIODCHECKIHM_MS 50
#define INPUTKEYDEBOUNCEMS 150


#define ADCLEVES 1023


static StateIHM g_menu_state = INITIAL_SCREEN;
static RespiratorInfo* gRespInfoPtr;
static RespiratorEstInfo* gRespEstPtr;
static unsigned long gtimescreennew_ms=0; // time used for screen
static unsigned long gtimescreenold_ms=0; // time used for screen



void setMenuState(StateIHM state)
{
	g_menu_state = state;
}
StateIHM getMenuState()
{
	return g_menu_state;
}

void ihm_init(RespiratorInfo* respinfoptr, RespiratorEstInfo *respestptr){
#ifndef LCD_128x64
/*
  lcd.createChar(0, lcd_char_ambu_up);
  lcd.createChar(1, lcd_char_ambu_down);
  lcd.begin(16, 2);
  */
#else
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  lcd_128x64.setFont(FONT_TYPE);
/*
//TODO: configre display
  Display.setFontRefHeightExtendedText();
  Display.setDefaultForegroundColor();
  Display.setFontPosTop();  */
#endif
  updatingScreen = 0;
  gRespInfoPtr = respinfoptr;
  gRespEstPtr = respestptr;
  pinMode(KEY_ALR_PIN,INPUT_PULLUP);
  pinMode(KEY_CFG_PIN,INPUT_PULLUP);
  pinMode(KEY_UP_PIN,INPUT_PULLUP);
  pinMode(KEY_OK_PIN,INPUT_PULLUP);
  pinMode(KEY_DOWN_PIN,INPUT_PULLUP);
  pinMode(KEY_STAND_BY_PIN,INPUT_PULLUP);
#ifdef RESET_LCD_OFTEN
  pinMode(LCD_128x64_RESET_PIN, OUTPUT); //
  digitalWrite(LCD_128x64_RESET_PIN, 1); //active
#endif

}
void delayButtonCheck(long time)
{
	last_time_button_check = millis() + time;
}
#ifdef RESET_LCD_OFTEN
 void resetLCD()
 {
	digitalWrite(LCD_128x64_RESET_PIN, 0); //disable
	u8g_10MicroDelay(); //delay(1000); 10us delay, according to datasheet
	digitalWrite(LCD_128x64_RESET_PIN, 1); //enable
	u8g_10MicroDelay(); //10us delay
	//delay(10000); //for tests purposes
	lcd_128x64.begin();
	requestScreenUpdate();
 }
#endif

Key ihm_get_key() {
	unsigned long current_time;
	Key returnKey = KEY_NOTPRESSED;
	current_time = millis();
	if(last_time_button_check == 0 || (current_time > last_time_button_check && (current_time-last_time_button_check) > INPUTKEYDEBOUNCEMS)){
		last_time_button_check = current_time; // update time for debounce

		// The input buttons are in inverted logic
		if(!digitalRead(KEY_STAND_BY_PIN)){
		  returnKey = KEY_STAND_BY;
		}
		if(!digitalRead(KEY_DOWN_PIN)){
		  returnKey = KEY_DOWN;
		}
		if(!digitalRead(KEY_OK_PIN)){
		  returnKey = KEY_OK;
		}
		if(!digitalRead(KEY_UP_PIN)){
		  returnKey = KEY_UP;
		}
		if(!digitalRead(KEY_CFG_PIN)){
		  returnKey = KEY_CFG;
		}
		if(!digitalRead(KEY_ALR_PIN)){
		  returnKey = KEY_ALR;
		}
	}
	//if there was a pressed key, wait twice the time
	if(returnKey != KEY_NOTPRESSED)
		last_time_button_check += INPUTKEYDEBOUNCEMS;
	return returnKey;
 }
/*
void menuPosChanged(int oldposE, int newposE){
  // LCD Output:
  // ****************
  // *N   |        (New config)
  // *A        |     (Old config)
  // ****************

#ifndef LCD_128x64

   // print old configuration
   lcd.setCursor(0,1);
   lcd.print(F("A               "));
   lcd.setCursor(oldposE,1);
   lcd.print(F("|"));

   // print new configuration
   lcd.setCursor(0,0);
   lcd.print(F("N               "));
   lcd.setCursor(newposE,0);
   lcd.print(F("|"));
   lcd.setCursor(newposE,0);
   lcd.blink();

   updatingScreen = 0;
#else
  lcd_128x64.drawStr(0, (LCD_HEIGHT-FONT_HEIGHT_P)/2, "N");
    lcd_128x64.drawStr(newposE*FONT_WIDTH, (LCD_HEIGHT-FONT_HEIGHT_P)/2, "|");

  lcd_128x64.drawStr(0, (LCD_HEIGHT+FONT_HEIGHT_P)/2, "A");
    lcd_128x64.drawStr(oldposE*FONT_WIDTH, (LCD_HEIGHT+FONT_HEIGHT_P)/2, "|");

  if(!lcd_128x64.nextPage())
    updatingScreen = 0;
#endif
}*/


/**
 * input pots
 * output bpm
 * output ie_ratio
 */
void convertPotsToVpmAndIeratio(PotsInfo pots, int* bpm, float* ie_ratio, int* volume){
  // Get potentionmeter data and implment Hysteresis
  int idx;
  static bool firstExecutation =true;

  static unsigned short currentOutputLevelBpm,currentOutputLevelIeratio,currentOutputLevelVolume;

  const short numberOfLevelsOutput = 21;

  // margin sets the 'stickyness' of the hysteresis or the relucatance to leave the current state.
  // It is measured in units of the the input level. As a guide it is a few percent of the
  // difference between two end points. Don't make the margin too wide or ranges may overlap.
  const unsigned short  margin = 17;   //  +/- 10

  // Both potentionmeter should be mapped to the same number of levels
  // Thus they can use the same endPointInput
  const unsigned short endPointInput[numberOfLevelsOutput+1] PROGMEM =
  {0,49,97,146,195,244,292,341,390,438,487,536,585,633,682,731,779,828,877,926,974,1023,};
  //{26,77,128,179,230,281,332,384,435,486,537,588,639,691,742,793,844,895,946,997};

  const float lut_ieratio[numberOfLevelsOutput] PROGMEM =
  {1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.6,2.7,2.8,2.9,3.0};

  const int lut_bpm[numberOfLevelsOutput] PROGMEM =
  {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};

  const int lut_volume[numberOfLevelsOutput] PROGMEM =
  {200,220,240,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600};

  unsigned short inputLevelBpm    = pots.potbpm;
  unsigned short inputLevelIeratio  = pots.potieratio;
  unsigned short inputLevelVolume  = pots.potvolume;

  if(firstExecutation){
    firstExecutation=false;
    // The first time the code is executed is different
    // no hysteresis is possible because there is no "last state"
    for(idx = 1;idx<numberOfLevelsOutput-1;++idx){
      if( inputLevelBpm>= endPointInput[idx] &&
        inputLevelBpm<=endPointInput[idx+1]){
        currentOutputLevelBpm = idx;
        break;
      }
    }

    for(idx = 1;idx<numberOfLevelsOutput-1;++idx){
      if( inputLevelIeratio>= endPointInput[idx] &&
        inputLevelIeratio <=endPointInput[idx+1]){
        currentOutputLevelIeratio = idx;
        break;
      }
    }


    // The first time the code is executed is different
    // no hysteresis is possible because there is no "last state"
    for(idx = 1;idx<numberOfLevelsOutput-1;++idx){
      if( inputLevelVolume>= endPointInput[idx] &&
        inputLevelVolume<=endPointInput[idx+1]){
        currentOutputLevelVolume = idx;
        break;
      }
    }
    *bpm    = lut_bpm[currentOutputLevelBpm];
    *ie_ratio = lut_ieratio[currentOutputLevelIeratio];
    *volume = VOLUME_INV_CALC(lut_volume[currentOutputLevelVolume]);
    return;
  }

  // Lower bound and upper bound for Hysteresis implementation for BPM
  // get lower and upper bounds for currentOutputLevel
  unsigned short lbBpm = endPointInput[ currentOutputLevelBpm ];
  if ( currentOutputLevelBpm > 0 )
    lbBpm -= margin;   // subtract margin

  unsigned short ubBpm = endPointInput[ currentOutputLevelBpm+1 ];
  if ( currentOutputLevelBpm < numberOfLevelsOutput )
    ubBpm +=  margin;  // add margin

  // Lower bound and upper bound for Hysteresis implementation for IE ratio
  // get lower and upper bounds for currentOutputLevel
  unsigned short lbIeratio= endPointInput[currentOutputLevelIeratio];
  if ( currentOutputLevelIeratio > 0 )
    lbIeratio -= margin;   // subtract margin

  unsigned short ubIeratio = endPointInput[ currentOutputLevelIeratio+1 ] ;
  if ( currentOutputLevelIeratio < numberOfLevelsOutput )
    ubIeratio +=  margin;  // add margin


  // Lower bound and upper bound for Hysteresis implementation for Volume
  // get lower and upper bounds for currentOutputLevel
  unsigned short lbVolume = endPointInput[ currentOutputLevelVolume ];
  if ( currentOutputLevelVolume > 0 )
    lbVolume -= margin;   // subtract margin

  unsigned short ubVolume = endPointInput[ currentOutputLevelVolume+1 ];
  if ( currentOutputLevelVolume < numberOfLevelsOutput )
    ubVolume +=  margin;  // add margin


  // now test if input is between the outer margins for current output value
  if ( inputLevelBpm < lbBpm || inputLevelBpm > ubBpm ) {
  // determine new output level by scanning endPointInput array
    for ( idx = 0 ; idx < numberOfLevelsOutput ; idx++ ) {
      if (inputLevelBpm >= endPointInput[ idx ] &&
        inputLevelBpm <= endPointInput[ idx + 1 ] )
        break ;
    }
        currentOutputLevelBpm = idx;
  }

  // now test if input is between the outer margins for current output value
  if ( inputLevelIeratio < lbIeratio || inputLevelIeratio > ubIeratio ) {
    // determine new output level by scanning endPointInput array
    for ( idx = 0 ; idx < numberOfLevelsOutput ; idx++ ) {
      if ( inputLevelIeratio >= endPointInput[ idx ] &&
         inputLevelIeratio <= endPointInput[ idx + 1 ] )
        break ;
    }
        currentOutputLevelIeratio = idx ;
  }

  // now test if input is between the outer margins for current output value
  if ( inputLevelVolume < lbVolume || inputLevelVolume > ubVolume ) {
  // determine new output level by scanning endPointInput array
    for ( idx = 0 ; idx < numberOfLevelsOutput ; idx++ ) {
      if (inputLevelVolume >= endPointInput[ idx ] &&
        inputLevelVolume <= endPointInput[ idx + 1 ] )
        break ;
    }
    currentOutputLevelVolume = idx;
  }



  *bpm    = lut_bpm[currentOutputLevelBpm];
  *ie_ratio = lut_ieratio[currentOutputLevelIeratio];
  *volume = VOLUME_INV_CALC(lut_volume[currentOutputLevelVolume]);
}

inline void menuPrintAlarmsIcons()
{
  char iconInd = 0;
#ifndef LCD_128x64
  char icons[ALARMTEXTSIZE] = " ";
  if(gActiveRespEst.alarms.activeAlarmsMask == 0 && gActiveRespEst.alarms.inactiveAlarmsMask == 0)
  {
#if DEBUG_CODE != 0
    txSerial.print(F("             "));
#endif
    return;
  }
#if DEBUG_CODE != 0
  txSerial.print(F(" ALRM: "));
#endif
#else
  if(gActiveRespEst.alarms.activeAlarmsMask == 0 && gActiveRespEst.alarms.inactiveAlarmsMask == 0)
      return;
  lcd_128x64.drawStr(3, FONT_HEIGHT_P*4+2, "ALRM: ");
  char icons[ALARMTEXTSIZE];
#endif

  for(int i = 1; i < ALARMTEXTSIZE; i++)
  {
    if(gActiveRespEst.alarms.activeAlarmsMask & 1 << (i-1))
    {
      icons[(int)iconInd] = 'A' + (i-1);
      iconInd++;
    } else if(gActiveRespEst.alarms.inactiveAlarmsMask & 1 << (i-1))
    {
      icons[(int)iconInd] = 'a' + (i-1);
      iconInd++;
    }
  }

#ifndef LCD_128x64
#if DEBUG_CODE != 0
  txSerial.print(icons);
#endif
#else
  icons[(int)iconInd] = 0;
  lcd_128x64.drawStr(3+FONT_WIDTH*6, FONT_HEIGHT_P*4+2, icons);
#endif
// */
}


void requestScreenUpdate()
{
#ifdef LCD_128x64
  lcd_128x64.firstPage();
#endif
  updatingScreen = 1;
  updateAlarmText();
}


const char messages[][22] PROGMEM  =  {  //MSG_NO_END_LIMIT_SWITCH:
												"Sem Chave de Exp.",
												//MSG_NO_START_LIMIT_SWITCH:
												"Sem Chave de Insp.",
												//MSG_STEP_LOSS_ERROR:
												"Erro no motor.",
												//MSG_START_IN_3S:
												"Inicia em 3s...",
												//MSG_CALIBRATING_ATM:
												"Calibrando...",
												//MSG_WRONG_RESTART_0:
												"Sistema desligado",
												//MSG_WRONG_RESTART_1:
												"incorretamente",
												//MSG_OK_TO_CONTINE:
												"OK para continuar",
												//MSG_MOVING_TEST_0:
												"TESTE DE MOVIMENTO",

												//MSG_CANCEL:
												"CFG para cancelar",
												//MSG_SYSTEM_OFF:
												"Pronto para Desligar",
												//MSG_INIT_MODE:
												"MODO DE INICIALIZAÇÃO",
												//MSG_OK_DEFAULT:
												"OK para modo padrão",
												//MSG_CFG_TEST:
												"CFG para testes",
												//MSG_VOL_CAL:
												"Calibrar Volume",
												//MSG_CALI_CONC:
												"Calibração Concluída",

												//MSG_MOVING_TEST_1:
												"O sistema executa mo-",
												//MSG_MOVING_TEST_2:
												"vimentos de abertura",
												//MSG_MOVING_TEST_3:
												"e fechamento",
												//MSG_PERFORMING_TESTS:
												"Teste em curso...",
												//MSG_TEST_CONCLUDED:
												"Teste concluído ",
												//MSG_TEST_SUCCESS:
												"com sucesso",
												//MSG_TEST_FAIL:
												"com falha.",
												//MSG_CFG_TO_REPEATE:
												"CFG para repetir",
												//MSG_ALARM_TEST_0:
												"TESTE DE ALARME",
												//MSG_ALARM_TEST_1:
												"O sistema emite",
												//MSG_ALARM_TEST_2:
												"sinais sonoros",
												//MSG_OK_LISTEN:
												"OK se escuta",
												//MSG_ALR_NOT_LISTEN:
												"ALR se não escuta",

												//INTRO_MSG_1
												"FASTEN Vita Pneuma",

												//MSG_ALR_SYS_ERROR:
												"ALR erro no sistema",

												//INTRO_MSG_2
												"Inicializando",
												//MSG_OF_POP_OFF:
												"de pop-off.",
												//MSG_ADJUST_IF_NEEDED:
												"Ajuste se necessário",
												//MSG_OBTAINED_PRESSURE:
												"Pressão Obtida(cmH2O)",

												//MSG_CAL_PRESS_0:
												"Calibrar o sensor de",
												//MSG_CAL_PRESS_1:
												"pressão com a pressão",
												//MSG_CAL_PRESS_2:
												"atmosférica.",
												//MSG_SYS_FAIL:
												"Falha no Sistema",
												//MSG_SHUT_DOWN:
												"Desligar equipamento?",
												//MSG_OK:
												"OK para sim",
												//MSG_PAUSED_SYSTEM:
												"Sistema em Pausa"
#if DEBUG_CODE != 0
												//DEBUG_MSG
												, "Modo Debug"
#endif
												};
/*
//steps used for pop-off test
//0 means no pop-off test.
#define STEP_POP_OFF_TEST_VOLUME 0//value in STEPs. 0 means no pop-off test.


#if STEP_POP_OFF_TEST_VOLUME > 0
	//MSG_OK_POP_OFF:
	"OK pop-off abriu",
#endif
#if STEP_POP_OFF_TEST_VOLUME > 0
	//MSG_POP_OFF_TEST_0:
	"TESTE DE POP-OFF",
	//MSG_POP_OFF_TEST_1:
	"Coloque o ambu na má-",
	//MSG_POP_OFF_TEST_2:
	"quina e feche a saída",
	//MSG_POP_OFF_TEST_3:
	"Lembre da pop-off",
#endif*/

//Print a single line according to its code.
//If the fixedMsg is set, the code is ignored and the fixedMsg is printed instead.
void printMessage(char msgID, unsigned char line, char *fixedMsg = NULL)
{

  int pos;
  const char *constMsg;
  //find the message
  if(msgID >= '1')
  	constMsg = messages[msgID - '1'];
//  else if(fixedMsg != NULL)
//  	constMsg = fixedMsg;
//  else //if(fixedMsg == NULL) //if the ID is invalid and there is no message
  else if(fixedMsg == NULL)
  	return;

  /*
  if(msgID >= 'a')
  	constMsg = messages[(int)msgID - (int)'a'];
  else if(msgID >= 'A')
  	constMsg = messages[(int)msgID - (int)'A'];
  else if(msgID >= '1')
  	constMsg = messages[(int)msgID - (int)'1'];
  else if(fixedMsg != NULL)
  	constMsg = fixedMsg;
  else //if(fixedMsg == NULL) //if the ID is invalid and there is no message
  	return;
  	*/


#ifndef LCD_128x64
#if DEBUG_CODE != 0
  if(fixedMsg != NULL)
	  txSerial.println(fixedMsg);
  else
	  txSerial.println(msg);
#endif
#else



	if(fixedMsg != NULL)
	{
		//Fixed pos. for now.
		pos = (LCD_WIDTH - (FONT_WIDTH*strlen(fixedMsg))) >> 1; //equals to theline bellow
		lcd_128x64.setPrintPos((unsigned char)pos, (unsigned char)(FONT_HEIGHT_P*line));
		lcd_128x64.print(fixedMsg);
	}
	else
	{
		pos = (LCD_WIDTH - (FONT_WIDTH*strlen_P(constMsg))) >> 1;
		lcd_128x64.setPrintPos((unsigned char)pos, (unsigned char)(FONT_HEIGHT_P*line));
		lcd_128x64.print((const __FlashStringHelper *)constMsg);
	}
#endif
}
void printMessageLine(char msg)
{
#ifdef LCD_128x64
  lcd_128x64.firstPage();
  do {
#endif
    printMessage(msg, 1 + (5.0f-1)/2);
#ifdef LCD_128x64
  } while(lcd_128x64.nextPage());
#endif
}
//Prints the lines passed in the msgs vector according to its code.
//It also allows to pass an optinal secondLine parameter, which overright the second line print
void printMessageLines(const char* msgs, unsigned char len, char* secondLine)
{
  unsigned char currLine;
#ifdef LCD_128x64
  lcd_128x64.firstPage();
  do {
#endif
    currLine = 1 + ((5.0f-len)/2);
    for(int i = 0; i < len; i++)
    {
	  //Should we print a fixed second line?
	  if(secondLine != NULL && i == 1)
	  	printMessage(msgs[i], currLine, secondLine);
	  else
      	printMessage(msgs[i], currLine);
      currLine++;
    }
#ifdef LCD_128x64
  } while(lcd_128x64.nextPage());
#endif
}

//WARNING: this is a blocking function
CmdIhm printMessageLinesWaitResp(const char* msgs, unsigned char len, char* secondLine)
{
	Key iKey;
	State systemState = getSystemState();

	//Do only call this function (and wait) when the system is not running breath cycles.
	if(systemState == S4 || systemState == S3)
	{
	  //do not wait here if the system is in a breath cycle
	  return NONE;
	}

	printMessageLines(msgs, len, secondLine);

	do
	{
		if(getWatchdogStatus())
			watchdog_reset();
		if(timer1_beepIsRunning())
			timer1_beep_update();
		iKey = ihm_get_key();

		switch(iKey)
		{
			case KEY_CFG:
			  return CONFIG;
			case KEY_STAND_BY:
			  return CMD_STAND_BY;
			case KEY_DOWN:
			  return CMD_DOWN;
			case KEY_OK:
			  return CMD_OK;
			case KEY_UP:
			  return CMD_UP;
			case KEY_ALR:
			  return CMD_ALR;
			default:
				//nothing to do here.
				break;
		}
	} while(iKey == KEY_NOTPRESSED);

	return NONE;
}

//WARNING: this is a blocking function
void printLastMessage(char msgID, bool beep)
{
	//this function may NEVER be called at S3 or S4 state
	if(getSystemState() > S2)
		return;
	printMessageLines(&msgID,1);
	if(beep)
		timer1_beep_start();
	while(1) {
		if(getWatchdogStatus())
			watchdog_reset();
		if(beep)
			timer1_beep_update();
	}
}
void defaultScreen(int bpm, float ie_ratio, int vol_pulses, char potChangedMask){
  // LCD Output
  /*****************
  *RPM  I:E    VOL.
  *30   1:4.0  100%
  *****************/
#ifndef LCD_128x64/*
  VOL_CHANGED = 1,
  BPM_CHANGED = 2,
  IE_CHANGED = 4
  int volume_est;
  char bpm_mea;
  float ie_ratio_mea;
  char assist_cur_breath; //0 for controled, 1 for assistied current breath
  float flux_est;   //current flux
  float pip_max_mea;
  float peep_min_mea; */
  //Withou LCD for debug for debug proposes
  #if DEBUG_CODE != 0
    txSerial.println(F(" "));
    txSerial.println(F("Df Screen"));
    if(potChangedMask & VOL_CHANGED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("VOL "));
    txSerial.print(VOLUME_CALC(vol_pulses));
    txSerial.print(F("/"));
    // Volume est is converted in to mL (if encessary) in s3.cpp
	txSerial.print(gRespEstPtr->volume_est);

    if(gRespInfoPtr->op_mode == AC_MODE)
      txSerial.println(F(" MD  A/C"));
    else if(gRespInfoPtr->op_mode == C_MODE)
      txSerial.println(F(" MD    C"));
    else if(gRespInfoPtr->op_mode == N_MODE)
      txSerial.println(F(" MD  N-R"));



    if(potChangedMask & BPM_CHANGED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("RPM  "));
    txSerial.print((int)bpm);
    txSerial.print(F("/"));
    txSerial.print((int)gRespEstPtr->bpm_mea);
    if(gRespEstPtr->assist_cur_breath)
      txSerial.println(F("  TRIGG A"));
    else
      txSerial.println(F("  TRIGG C"));


    if(potChangedMask & IE_CHANGED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("I-E 1:"));
    txSerial.print(ie_ratio);
    txSerial.print(F("/"));
    txSerial.println(gRespEstPtr->ie_ratio_mea);


    txSerial.print(F("             "));
    txSerial.print(F("FLX "));
    txSerial.println(gRespEstPtr->flux_est);

#ifdef USE_PRESSURE_SENSOR
    if(gRespInfoPtr->op_mode != N_MODE)
    {
      menuPrintAlarmsIcons();
      txSerial.print(F("PIP "));
      txSerial.println(gRespEstPtr->pip_max_mea);

      txSerial.print(F(" "));
      txSerial.print(alarmTexts[gRespEstPtr->alarms.alarmTextIndex]);

      txSerial.print(F(" PEEP "));
      txSerial.println(gRespEstPtr->peep_min_mea);
    }
#endif

  #endif

  updatingScreen = 0;
#else
  char str[21];

  lcd_128x64.drawFrame(0, 0, FONT_WIDTH*12 + 3 + 1, FONT_HEIGHT_P*3+2);


  //VOL
  if(potChangedMask & VOL_CHANGED)  {
#ifdef VOLUME_USE_ML
    lcd_128x64.drawBox(2, 2, (FONT_WIDTH*6)+1, FONT_HEIGHT_P);
#else
    lcd_128x64.drawBox(2, 2, (FONT_WIDTH*7)+1, FONT_HEIGHT_P);
#endif
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(2, FONT_HEIGHT_P*1, "VOL"); //line 1, col 0
  //lcd_128x64.drawStr(1+FONT_WIDTH*3-2, FONT_HEIGHT_P*1, ":"); //line 1, col 3
  itoa((VOLUME_CALC(vol_pulses)), str, 10);
  lcd_128x64.drawStr(FONT_WIDTH*4-3, FONT_HEIGHT_P*1, str); //line 1
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel

#ifdef VOLUME_USE_ML
  lcd_128x64.drawStr(FONT_WIDTH*7-3, FONT_HEIGHT_P*1, "/"); //line 1
  itoa(gRespEstPtr->volume_est, str, 10);
  lcd_128x64.drawStr(FONT_WIDTH*8-3, FONT_HEIGHT_P*1, str); //line 1
  lcd_128x64.drawStr(FONT_WIDTH*11-3, FONT_HEIGHT_P*1, "mL"); //line 1
#else
  lcd_128x64.drawStr(FONT_WIDTH*8-3, FONT_HEIGHT_P*1, "/"); //line 1
  itoa(gRespEstPtr->volume_est, str, 10);
  lcd_128x64.drawStr(FONT_WIDTH*9-3, FONT_HEIGHT_P*1, str); //line 1
#endif

  //BPM
  if(potChangedMask & BPM_CHANGED)  {
    lcd_128x64.drawBox(2, 2+FONT_HEIGHT_P*1, 1+(FONT_WIDTH*8), FONT_HEIGHT_P);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(3, FONT_HEIGHT_P*2, "RPM"); //line 1, col 0
  itoa(bpm, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*6, FONT_HEIGHT_P*2, str); //line 2
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  lcd_128x64.drawStr(3+FONT_WIDTH*8, FONT_HEIGHT_P*2, "/"); //line 2
  itoa(gRespEstPtr->bpm_mea, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*9, FONT_HEIGHT_P*2, str); //line 2

  //IE
  if(potChangedMask & IE_CHANGED)  {
    lcd_128x64.drawBox(2, 2+FONT_HEIGHT_P*2, 1+(FONT_WIDTH*8), FONT_HEIGHT_P);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(3, FONT_HEIGHT_P*3, "I-E"); //line 1, col 0
  lcd_128x64.drawStr(3+FONT_WIDTH*4-3, FONT_HEIGHT_P*3, "1"); //line 3
  lcd_128x64.drawStr(3+FONT_WIDTH*4+1, FONT_HEIGHT_P*3-2, ":"); //line 3
  dtostrf(ie_ratio, 3, 1, str);
  lcd_128x64.drawStr(3+FONT_WIDTH*5, FONT_HEIGHT_P*3, str); //line 3
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  lcd_128x64.drawStr(3+FONT_WIDTH*8, FONT_HEIGHT_P*3, "/"); //line 3
  dtostrf(gRespEstPtr->ie_ratio_mea, 3, 1, str);
  lcd_128x64.drawStr(3+FONT_WIDTH*9, FONT_HEIGHT_P*3, str); //line 3

  //Alarms
  lcd_128x64.drawFrame(0, FONT_HEIGHT_P*3+3, FONT_WIDTH*12 + 3 + 1, FONT_HEIGHT_P*2+1);
  menuPrintAlarmsIcons();
  lcd_128x64.drawStr(3, FONT_HEIGHT_P*5+2, alarmTexts[(int)gRespEstPtr->alarms.alarmTextIndex]);


  //Mode/Trigger
  lcd_128x64.drawFrame(FONT_WIDTH*12 + 3 + 2, 0, LCD_WIDTH-(FONT_WIDTH*12 + 3 + 2), FONT_HEIGHT_P*2+2);
  if(gRespInfoPtr->op_mode == AC_MODE)
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*1, "MD A/C");
  else if(gRespInfoPtr->op_mode == C_MODE)
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*1, "MD   C");
  else
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*1, "MD N-R");
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel


  if(gRespEstPtr->assist_cur_breath)
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*2, "TRIGG A");
  else
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*2, "TRIGG C");


  //FONT_WIDTH*12 + 3 + 4
  //Flux, PIP and PEEP est
  lcd_128x64.drawFrame(FONT_WIDTH*12 + 3 + 2, FONT_HEIGHT_P*2+2+1, LCD_WIDTH-(FONT_WIDTH*12 + 3 + 2), LCD_HEIGHT-(FONT_HEIGHT_P*2+2+1));
  lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*3+2, "FLX");

  //gRespEstPtr->flux_est =15.4f;
  dtostrf(gRespEstPtr->flux_est, 3, 1, str);
  lcd_128x64.drawStr(FONT_WIDTH*18-5, FONT_HEIGHT_P*3+2, str); //

#ifdef USE_PRESSURE_SENSOR
  if(gRespInfoPtr->op_mode != N_MODE)
  {
    //gRespEstPtr->pip_max_mea =15.4f;
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*4+2, "PIP");
    dtostrf(gRespEstPtr->pip_max_mea, 3, 1, str);
    lcd_128x64.drawStr(FONT_WIDTH*18-5, FONT_HEIGHT_P*4+2, str); //

    //gRespEstPtr->peep_min_mea =5.4f;
    lcd_128x64.drawStr(FONT_WIDTH*14-5, FONT_HEIGHT_P*5+2, "PEEP");
    dtostrf(gRespEstPtr->peep_min_mea, 3, 1, str);
    if(gRespEstPtr->peep_min_mea >= 0)
    	lcd_128x64.drawStr(FONT_WIDTH*19-5, FONT_HEIGHT_P*5+2, str); //
   	else
    	lcd_128x64.drawStr(FONT_WIDTH*18-5, FONT_HEIGHT_P*5+2, str); //
  }
#endif
  if(!lcd_128x64.nextPage())
    updatingScreen = 0;
#endif
}


inline CmdIhm defaultScreenFunc(Key key, PotsInfo pot)
{
  static bool firstTime = true;
  int new_volume;
  int new_bpm;
  float new_ie_ratio;
  static int old_volume = -1;
  static int old_bpm = -1;
  static float old_ie_ratio = -1;
  static char potChangedMask = 0;
  bool input_data_changed=false;

  //if it is the first time, we call convertPotsToVpmAndIeratio twice
  if(firstTime)
    convertPotsToVpmAndIeratio(pot, &old_bpm, &old_ie_ratio, &old_volume);

  convertPotsToVpmAndIeratio(pot, &new_bpm, &new_ie_ratio, &new_volume);
  //potChangedMask |= (new_volume != old_volume)*VOL_CHANGED + (new_bpm != old_bpm)*BPM_CHANGED + (new_ie_ratio != old_ie_ratio)*IE_CHANGED;
  potChangedMask = (new_volume != gRespInfoPtr->volume_pulses)*VOL_CHANGED + (new_bpm != gRespInfoPtr->bpm)*BPM_CHANGED + (new_ie_ratio != gRespInfoPtr->ie_ratio)*IE_CHANGED;
  input_data_changed = (new_volume != old_volume) || (new_bpm != old_bpm) || (new_ie_ratio != old_ie_ratio); //must compare against old values to update any screen changes.
  old_bpm = new_bpm;
  old_ie_ratio = new_ie_ratio;
  old_volume = new_volume;

  if(firstTime) {
    firstTime = false;
    potChangedMask = 0;
    input_data_changed = true;
  }


  //if we are updating the screen and there is not an input change (that causes a screen update anyway)
  if(updatingScreen && !input_data_changed)
  {
    if(potChangedMask == 0)
      defaultScreen(gRespInfoPtr->bpm, gRespInfoPtr->ie_ratio, gRespInfoPtr->volume_pulses, potChangedMask);
    else
      defaultScreen(new_bpm, new_ie_ratio, new_volume, potChangedMask);
  }
  //delay any futher button press
  if(key != KEY_NOTPRESSED)
	delayButtonCheck(500);

#ifdef USE_PRESSURE_SENSOR
  if((KEY_UP==key || KEY_DOWN==key) && gRespInfoPtr->op_mode != N_MODE) {
    g_menu_state = GRAPH_SCREEN;
    firstTime = true;
    return NONE;
  } else
#endif
  if(KEY_OK==key){
    //Save the config if there is something to be saved
    if(potChangedMask)
    {
      gRespInfoPtr->bpm = new_bpm;
      gRespInfoPtr->ie_ratio = new_ie_ratio;
      gRespInfoPtr->volume_pulses = new_volume;
      potChangedMask = 0;
      requestScreenUpdate();
      return NEWDATA;
    } else {
      //inform that the ok button was pressed
      return CMD_OK;
    }
  }

#ifdef USE_PRESSURE_SENSOR
  if(KEY_CFG==key){
    g_menu_state = CONFIG_SCREEN;
    firstTime = true;
    return NONE;
  }
#endif //USE_PRESSURE_SENSOR
  if(input_data_changed) {
    input_data_changed = false;
    requestScreenUpdate();
    if(potChangedMask == 0)
      defaultScreen(gRespInfoPtr->bpm, gRespInfoPtr->ie_ratio, gRespInfoPtr->volume_pulses, potChangedMask);
    else
      defaultScreen(new_bpm, new_ie_ratio, new_volume, potChangedMask);
    gtimescreenold_ms = millis();
  }

  switch(key)
  {
    case KEY_STAND_BY:
      return CMD_STAND_BY;
    case KEY_ALR:
      return CMD_ALR;
    default:
      return NONE;
  }
}


#ifdef USE_PRESSURE_SENSOR

void graphScreen(){
  // LCD Output
  /*****************
  *RPM  I:E    VOL.
  *30   1:4.0  100%
  *****************/
#ifndef LCD_128x64
  //Withou LCD for debug for debug proposes
  #if DEBUG_CODE != 0
    txSerial.println(F(" "));
    txSerial.println(F("Grp Scr"));

    txSerial.print(F("VOL: "));
    txSerial.print(gRespEstPtr->volume_est);


    txSerial.print(F(" BPM: "));
    txSerial.print((int)gRespEstPtr->bpm_mea);

    txSerial.print(F(" I-E 1:"));
    txSerial.println(gRespEstPtr->ie_ratio_mea);

  #endif

  updatingScreen = 0;
#else

  //lcd_128x64.setFont(u8g_font_helvR08);
  char str[21];
  //Cant use PROGMEM
  //Note that draw function prints less significant nibble  first, thus:
  // F5 = 1111 0101 will print 1010 1111
  unsigned char dotLine[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                                 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                                 0x55, 0x55, 0x55, 0x55, 0x55 };
  //static unsigned char currentLine[LINE_MEM_WIDTH], prevLine[LINE_MEM_WIDTH];
  /*lcd_128x64.drawXBM( 0, 10, LCD_WIDTH, 1, dotLine); //40cmH2O
  lcd_128x64.drawXBM( 0, 10+20, LCD_WIDTH, 1, dotLine); //20cmH2O
  lcd_128x64.drawXBM( 0, 10+40, LCD_WIDTH, 1, dotLine); //0cmH2O*/
  lcd_128x64.drawVLine(1, 10, 44);

/// *
  unsigned char printLine[LINE_MEM_WIDTH], currentLine[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  //in order to print the lines properly, there is -1 offset in the compValue.
  char currentCompValue = -1;
  char newByteValue = 0;
  int byteIndex;
  bool optimizeLoop = false;
  //TODO: it will be necessary to separate the vetors from timer1 and ihm, thus doubling the required memory
  //Go through all the lines
  /// *
  for(int i = 1; i <= HEIGHT_GRAPH_PRESSURE_BUFFER; i++)
  {/// *
    //we start from the current last position of the buffer
    byteIndex=gBufferPrintIdx;

    if(optimizeLoop == false)
    {
      optimizeLoop = true;
    } else {
      //if so, just print the dot lines
      if(i == 44 || i == 24 || i == 4)
        lcd_128x64.drawXBM( 0, 10+HEIGHT_GRAPH_PRESSURE_BUFFER-1-i, LCD_WIDTH, 1, dotLine);
      continue;
    }
    //Go through all the bytes of the line
    for(int j = 0; j < LINE_MEM_WIDTH; j++)
    {
      //8 bits of the current value
      newByteValue = 0;

      //We should do the calcs only if this 8 cols are still with data
      if(currentLine[j] != 0)
      {
        for(int k = 0; k < 8; k++)
        {
          if(gPrintPsensor[byteIndex] == currentCompValue)
            newByteValue |= 1<<k;
            /*
          //update the index of the buffer
          byteIndex++;
          if(byteIndex >= LENGTH_GRAPH_PRESSURE_BUFFER)
          {
            byteIndex = 0;
          }*/
          /// *
          byteIndex--;
          if(byteIndex < 0)
          {
            byteIndex = LENGTH_GRAPH_PRESSURE_BUFFER-1;
          }// */
        }
        currentLine[j] ^= newByteValue;
        //currentLine[j] = !currentLine[j] && (currentLine[j] ^ newByteValue);
        printLine[j] = currentLine[j];
        //if optimize the loop is still on, we should turn it off
        if(optimizeLoop)
          optimizeLoop = false;
      } else {
        printLine[j] = 0;
        byteIndex -= 8;
        if(byteIndex < 0)
          byteIndex += LENGTH_GRAPH_PRESSURE_BUFFER;
      }
      if(i == 44 || i == 24 || i == 4)
      {
        printLine[j] ^= dotLine[j];
      }
    }
    //printLine[0] |= 1 << 0;
    lcd_128x64.drawXBM( 0, 10+HEIGHT_GRAPH_PRESSURE_BUFFER-i, LCD_WIDTH, 1, printLine); //0cmH2O

    currentCompValue++;
  }// */


  //VOL
  lcd_128x64.drawStr(0, FONT_HEIGHT, "VOL"); //line 1, col 0
  itoa(gRespEstPtr->volume_est, str, 10);
  lcd_128x64.drawStr(FONT_WIDTH*4-3, FONT_HEIGHT, str); //line 0

  //BPM
  lcd_128x64.drawStr(FONT_WIDTH*8-4, FONT_HEIGHT, "BPM"); //line 0
  itoa(gRespEstPtr->bpm_mea, str, 10);
  lcd_128x64.drawStr(FONT_WIDTH*12-7, FONT_HEIGHT, str); //line 0

  //IE
  lcd_128x64.drawStr(FONT_WIDTH*15-8, FONT_HEIGHT, "I-E"); //line 0
  lcd_128x64.drawStr(FONT_WIDTH*19-12, FONT_HEIGHT, "1"); //line 0
  lcd_128x64.drawStr(FONT_WIDTH*19-8, FONT_HEIGHT-1, ":"); //line 0
  dtostrf(gRespEstPtr->ie_ratio_mea, 3, 1, str);
  lcd_128x64.drawStr(FONT_WIDTH*19-3, FONT_HEIGHT, str); //line 0

  lcd_128x64.drawStr(0, LCD_HEIGHT, "DX 5 SEG  DY 20 CMH2O"); //last line
  lcd_128x64.drawStr(FONT_WIDTH*2-1, LCD_HEIGHT-1, ":"); //last line
  lcd_128x64.drawStr(FONT_WIDTH*12-1, LCD_HEIGHT-1, ":"); //last line

  if(!lcd_128x64.nextPage())
  {
    //delay(10000);
    updatingScreen = 0;
  }
#endif
}




inline CmdIhm graphScreenFunc(Key key)
{
  static bool firstTime = true;

  if(firstTime) {
#ifdef LCD_128x64
    //copy the graph buffer current state
    if(!updatingScreen)
      gBufferPrintIdx = copyGraphBuffer(gPrintPsensor);
#endif
    gtimescreenold_ms = millis();
    updatingScreen = true;
    firstTime = false;
  }

  //delay any futher button press
  if(key != KEY_NOTPRESSED)
	delayButtonCheck(500);

  gtimescreennew_ms = millis();
#ifdef LCD_128x64
  if((gtimescreennew_ms-gtimescreenold_ms) > GRAPH_UPDATE_MS && !updatingScreen){
    gtimescreenold_ms = gtimescreennew_ms;
    gBufferPrintIdx = copyGraphBuffer(gPrintPsensor);
    requestScreenUpdate();
  }
#endif
  if((KEY_UP==key || KEY_DOWN==key) && gRespInfoPtr->op_mode != N_MODE) {
    g_menu_state = DEFAULT_SCREEN;
    firstTime = true;
    return NONE;
  } else
  if(KEY_OK==key){
    //inform that the ok button was pressed
    return CMD_OK;
  } else if(KEY_CFG==key){
    g_menu_state = CONFIG_SCREEN;
    firstTime = true;
    return NONE;
  }

  //if we are updating the screen
  if(updatingScreen)
    graphScreen();

  switch(key)
  {
    case KEY_STAND_BY:
      return CMD_STAND_BY;
    case KEY_ALR:
      return CMD_ALR;
    default:
      return NONE;
  }
}

inline void menuConfigScreen(char new_pip_max, char new_peep_min, float new_trigger, char op_mode, int selectedConfig)
{
#ifndef LCD_128x64
  //Withou LCD for debug for debug proposes
  #if DEBUG_CODE != 0
    txSerial.println(F(" "));
    txSerial.println(F("Menu Config Screen"));
    txSerial.println(F(" AJUSTE  OS  SETPOINTS"));
    txSerial.println(F("               MIN MAX"));

    if(selectedConfig == PIP_SELECTED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("PIP MAX  "));
    txSerial.print((int)new_pip_max);
    txSerial.print(F("   "));
    txSerial.print((int)MIN_PIP);
    txSerial.print(F("  "));
    txSerial.println((int)MAX_PIP);


    if(selectedConfig == PEEP_SELECTED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("PEEP MIN "));
    txSerial.print((int)new_peep_min);
    txSerial.print(F("   "));
    txSerial.print((int)MIN_PEEP_MIN);
    txSerial.print(F("  "));
    txSerial.println((int)MAX_PEEP_MIN);


    if(selectedConfig == TRIGGER_SELECTED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    txSerial.print(F("TRIGGER  "));
    txSerial.print(new_trigger);
    txSerial.print(F(" "));
    txSerial.print((int)MIN_TRIGGER);
    txSerial.print(F("  "));
    txSerial.println((int)MAX_TRIGGER);

    if(selectedConfig == MD_SELECTED)
      txSerial.print(F(">"));
    else
      txSerial.print(F(" "));
    if(op_mode == AC_MODE)
      txSerial.println(F("MODO A/C"));
    else if(op_mode == C_MODE)
      txSerial.println(F("MODO C"));
    else if(op_mode == N_MODE)
      txSerial.println(F("MODO N-R"));

  #endif
  updatingScreen = 0;
#else
  char str[21];

  //lcd_128x64.setFont(FONT_TYPE);
  //static unsigned char str_ADJUST[] U8G_PROGMEM = "AJUSTE OS SETPOINTS:";
  lcd_128x64.drawStr(FONT_WIDTH, FONT_HEIGHT, "AJUSTE OS SETPOINTS:");
  lcd_128x64.drawFrame(0, FONT_HEIGHT+1, LCD_WIDTH, LCD_HEIGHT-(FONT_HEIGHT+1));
  lcd_128x64.drawStr(FONT_WIDTH*13, FONT_HEIGHT_P_M*2+1, "MIN MAX"); //line 2, col 13

  //PIP
  if(selectedConfig == PIP_SELECTED)
  {
    lcd_128x64.drawBox(2, FONT_HEIGHT_P_M*2+1, LCD_WIDTH-4, FONT_HEIGHT_P_M+1);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*3+1, "PIP MAX"); //line 3
  itoa(new_pip_max, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*9, FONT_HEIGHT_P_M*3+1, str); //line 3, col 9
  itoa(MIN_PIP, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*13, FONT_HEIGHT_P_M*3+1, str); //line 3, col 13
  itoa(MAX_PIP, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*17, FONT_HEIGHT_P_M*3+1, str); //line 3, col 17

  //PEEP
  if(selectedConfig == PEEP_SELECTED)
  {
    lcd_128x64.drawBox(2, FONT_HEIGHT_P_M*3+1, LCD_WIDTH-4, FONT_HEIGHT_P_M+1);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  } else {
    lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*4+1, "PEEP MIN"); //line 4
  itoa(new_peep_min, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*9, FONT_HEIGHT_P_M*4+1, str); //line 4
  itoa(MIN_PEEP_MIN, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*13, FONT_HEIGHT_P_M*4+1, str); //line 4, col 13
  itoa(MAX_PEEP_MIN, str, 10);
  lcd_128x64.drawStr(3+FONT_WIDTH*17, FONT_HEIGHT_P_M*4+1, str); //line 4, col 17

  //Trigger
  if(selectedConfig == TRIGGER_SELECTED)
  {
    lcd_128x64.drawBox(2, FONT_HEIGHT_P_M*4+1, LCD_WIDTH-4, FONT_HEIGHT_P_M+1);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  } else {
    lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  }
  lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*5+1, "TRIGGER"); //line 5
  dtostrf(new_trigger, 3, 1, str);
  lcd_128x64.drawStr(3+FONT_WIDTH*9, FONT_HEIGHT_P_M*5+1, str); //line 5
  dtostrf(MIN_TRIGGER, 3, 1, str);
  lcd_128x64.drawStr(3+FONT_WIDTH*13, FONT_HEIGHT_P_M*5+1, str); //line 5, col 13
  dtostrf(MAX_TRIGGER, 3, 1, str);
  lcd_128x64.drawStr(3+FONT_WIDTH*17, FONT_HEIGHT_P_M*5+1, str); //line 5, col 17
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel



  //Trigger
  if(selectedConfig == MD_SELECTED)
  {
    lcd_128x64.drawBox(2, FONT_HEIGHT_P_M*5+1, LCD_WIDTH-4, FONT_HEIGHT_P_M+1);
    lcd_128x64.setColorIndex(0); //set (1) or clear (0) pixel
  } else {
    lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel
  }
  if(op_mode == AC_MODE)
    lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*6+1, "MODO A/C");
  else if(op_mode == C_MODE)
    lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*6+1, "MODO C");
  else if(op_mode == N_MODE)
    lcd_128x64.drawStr(3, FONT_HEIGHT_P_M*6+1, "MODO N-R");
  lcd_128x64.setColorIndex(1); //set (1) or clear (0) pixel

  if(!lcd_128x64.nextPage())
    updatingScreen = 0;
#endif
}

inline CmdIhm configScreenFunc(Key key)
{
  static int selectedConfig = PIP_SELECTED;
  static bool firstTime = true;
  static char new_pip_max, new_peep_min;
  static float new_trigger;
  static char new_op_mode;
  bool input_data_changed=false;

  if(firstTime) {
    //Obtain the current values
    new_pip_max = gRespInfoPtr->pip_max;
    new_peep_min = gRespInfoPtr->peep_min;
    new_trigger = gRespInfoPtr->trigger;
    new_op_mode = gRespInfoPtr->op_mode;
    // Start counting time
    gtimescreenold_ms = millis();
    input_data_changed = true;
    firstTime = false;
    //set the initial config.
    selectedConfig = PIP_SELECTED;
    requestScreenUpdate();
  }
  //if we are updating the screen and there is not an input change (that causes a screen update anyway)
  if(updatingScreen && !input_data_changed)
    menuConfigScreen(new_pip_max, new_peep_min, new_trigger, new_op_mode, selectedConfig);

  /* gtimescreennew_ms = millis();
  if((gtimescreennew_ms-gtimescreenold_ms)>TIMESCREEN_MS){
    g_menu_state = DEFAULT_SCREEN;
    // we are leaving this state, so the next time it enters here
    // it will  be the first time on this menu
    firstTime = true;
    return CONFIG;
  //if the operator pressed the config key, or if he pressed ok and we are at the last value (trigger)
  } else */

  if(key == KEY_CFG || (key == KEY_OK && selectedConfig == MD_SELECTED)) {
	delayButtonCheck(500);
    //update the current values
    gRespInfoPtr->pip_max = new_pip_max;
    gRespInfoPtr->peep_min = new_peep_min;
    gRespInfoPtr->trigger = new_trigger;
    gRespInfoPtr->op_mode  = new_op_mode;
    g_menu_state = DEFAULT_SCREEN;
    // we are leaving this state, so the next time it enters here
    // it will  be the first time on this menu
    firstTime = true;
    return NEWDATA;
  } else if(KEY_OK==key) {
	delayButtonCheck(500);
    //note that, if selectedConfig == MD_SELECTED, the else if above would return true;
    selectedConfig = selectedConfig + 1;
    input_data_changed = true;
  } else {
    if((KEY_UP==key || KEY_DOWN==key)){
      switch(selectedConfig)
      {
        case PIP_SELECTED:
          if(KEY_UP==key && (new_pip_max < MAX_PIP))
            new_pip_max += INC_PIP;
          else if(KEY_DOWN==key && (new_pip_max > MIN_PIP))
            new_pip_max -= INC_PIP;
        break;
        case PEEP_SELECTED:
          if(KEY_UP==key && (new_peep_min < MAX_PEEP_MIN))
            new_peep_min += INC_PEEP_MIN;
          else if(KEY_DOWN==key && (new_peep_min > MIN_PEEP_MIN))
            new_peep_min -= INC_PEEP_MIN;
        break;
        case TRIGGER_SELECTED:
          if(KEY_UP==key && ((float)new_trigger+INC_TRIGGER <= (float)MAX_TRIGGER))
            new_trigger += INC_TRIGGER; //TODO: BUG: the max value isn't respected
          else if(KEY_DOWN==key && (new_trigger > MIN_TRIGGER))
            new_trigger -= INC_TRIGGER;
        break;
        case MD_SELECTED:
          if(KEY_UP==key)
          {
            if(new_op_mode == N_MODE)
              new_op_mode = AC_MODE;
            else
              new_op_mode++;
          }
          else if(KEY_DOWN==key)
          {
            if(new_op_mode == AC_MODE)
              new_op_mode = N_MODE;
            else
              new_op_mode--;
          }
        break;
      }
      input_data_changed = true;
    }
  }
  if(input_data_changed) {
    requestScreenUpdate();
    menuConfigScreen(new_pip_max, new_peep_min, new_trigger, new_op_mode, selectedConfig);
    gtimescreenold_ms = millis();
  }

  switch(key)
  {
    case KEY_STAND_BY:
      return CMD_STAND_BY;
//    case KEY_DOWN: //Down, Ok, and Up are dealt inside this function and should not be returned.
//      return CMD_DOWN;
//    case KEY_OK:
//      return CMD_OK;
//    case KEY_UP:
//      return CMD_UP;
    case KEY_ALR:
      return CMD_ALR;
    default:
      return NONE;
  }
}
#endif //USE_PRESSURE_SENSOR


void popoffInfoScreen(float pressure)
{
  	char str[21];

#ifndef LCD_128x64
	//Withou LCD for debug for debug proposes
	#if DEBUG_CODE != 0
#ifdef USE_PRESSURE_SENSOR
		txSerial.print(F("Pressão obtida: "));
		txSerial.println(pressure);
#endif
		txSerial.println(F("Verifique se "));
		txSerial.println(F("a válvula abriu."));
	#endif
	updatingScreen = 0;
#else
	lcd_128x64.firstPage();
	do {
#ifdef USE_PRESSURE_SENSOR
		lcd_128x64.setPrintPos((unsigned char)0, (unsigned char)(FONT_HEIGHT_P*2));
		lcd_128x64.print(F("Pressão obtida:"));
  		dtostrf(pressure, 3, 1, str);
  		lcd_128x64.drawStr(FONT_WIDTH*16, (FONT_HEIGHT_P*2), str);
#endif

		lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*4.5f), (unsigned char)(FONT_HEIGHT_P*3));
		lcd_128x64.print(F("Verifique se"));
		lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*2.5f), (unsigned char)(FONT_HEIGHT_P*4));
		lcd_128x64.print(F("a válvula abriu."));

	} while(lcd_128x64.nextPage());
	updatingScreen = 0;

#endif
}
/*
void popoffScreen(int subscreen)
{
	if(subscreen == 0)
	{
		#ifndef LCD_128x64
		  //Withou LCD for debug for debug proposes
		  #if DEBUG_CODE != 0
			txSerial.println(F("Trave a saída de ar "));
			txSerial.println(F("e pressione Cfg para"));
			txSerial.println(F("verifica a válvula de"));
			txSerial.println(F("pop-off. Para pular, "));
			txSerial.println(F("pressione Ok."));
		  #endif
		   updatingScreen = 0;
		#else
		  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*1));
		  lcd_128x64.print(F("Trave a saída de ar "));
		  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*2));
		  lcd_128x64.print(F("e pressione Cfg para"));
		  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*3));
		  lcd_128x64.print(F("verifica a válvula de"));
		  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*4));
		  lcd_128x64.print(F("pop-off. Para pular, "));
		  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*5));
		  lcd_128x64.print(F("pressione Ok."));

		  if(!lcd_128x64.nextPage())
			updatingScreen = 0;
		#endif
	}
}
inline CmdIhm popoffScreenFunc(Key key, PotsInfo pot, bool debounce_ok)
{
  static bool firstTime = true;
  static int subscreen = 0;
  if(firstTime) {
    requestScreenUpdate();
    firstTime = false;
    subscreen = 0;
  }

  //If Ok is pressed, we must move to the config screen
  if(key == KEY_OK) {
#ifdef USE_PRESSURE_SENSOR
    g_menu_state = CONFIG_SCREEN; //shows the config screen
#else
    g_menu_state = DEFAULT_SCREEN; //shows the default screen
#endif
    firstTime = true;
    subscreen = 0;
    return CMD_OK;
  }
  if(key == KEY_CFG) {
    subscreen = 1;
    return CONFIG;
  }

  //if we are updating the screen
  if(subscreen == 0 && updatingScreen)
    popoffScreen(subscreen);

  return NONE;
}
*/

/*
void caliScreen()
{
	#ifndef LCD_128x64
	  //Withou LCD for debug for debug proposes
	  #if DEBUG_CODE != 0
		txSerial.println(F("Pressione Cfg para"));
		txSerial.println(F("calibrar o sensor de"));
		txSerial.println(F("pressão com a pressão "));
		txSerial.println(F("atmosférica. Para "));
		txSerial.println(F("pular, pressione Ok."));
	  #endif
	   updatingScreen = 0;
	#else
	  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*1));
	  lcd_128x64.print(F("Pressione Cfg para"));
	  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*2));
	  lcd_128x64.print(F("calibrar o sensor de"));
	  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*3));
	  lcd_128x64.print(F("pressão com a pressão "));
	  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*4));
	  lcd_128x64.print(F("atmosférica. Para "));
	  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*5));
	  lcd_128x64.print(F("pular, pressione Ok."));

	  if(!lcd_128x64.nextPage())
		updatingScreen = 0;
	#endif
}
inline CmdIhm caliScreenFunc(Key key, PotsInfo pot, bool debounce_ok)
{
  static bool firstTime = true;
  if(firstTime) {
    requestScreenUpdate();
    firstTime = false;
  }

  //If Ok is pressed, we must move to the initial screen
  if(key == KEY_OK) {
    g_menu_state = INITIAL_SCREEN; //shows the initial screen
    firstTime = true;
    return CMD_OK;
  }
  //if config is pressed, we must indicated that so the calibration can be done.
  if(key == KEY_CFG) {
    g_menu_state = INITIAL_SCREEN; //shows the initial screen
    firstTime = true;
    return CONFIG;
  }

  //if we are updating the screen
  if(updatingScreen)
    caliScreen();

  return NONE;
}
*/

/*
void beepScreen()
{

#ifndef LCD_128x64
  //Withou LCD for debug for debug proposes
  #if DEBUG_CODE != 0
    txSerial.println(F("O beep está tocando."));
    txSerial.println(F("Pressione Ok se você"));
    txSerial.println(F("estiver ouvindo ou se"));
    txSerial.println(F("desejar continuar "));
    txSerial.println(F("sem som de alarme."));
  #endif
   updatingScreen = 0;
#else
  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*1));
  lcd_128x64.print(F("O beep está tocando."));
  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*2));
  lcd_128x64.print(F("Pressione Ok se você"));
  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*3));
  lcd_128x64.print(F("estiver ouvindo ou se"));
  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*4));
  lcd_128x64.print(F("desejar continuar "));
  lcd_128x64.setPrintPos(0, (unsigned char)(FONT_HEIGHT_P*5));
  lcd_128x64.print(F("sem som de alarme."));


  if(!lcd_128x64.nextPage())
    updatingScreen = 0;
#endif
}
inline CmdIhm beepScreenFunc(Key key, PotsInfo pot, bool debounce_ok)
{
  static bool firstTime = true;

  if(firstTime) {
    requestScreenUpdate();
    firstTime = false;
  }

  if(key == KEY_OK) {
    g_menu_state = POP_OFF_SCREEN;
    firstTime = true;
    return CMD_OK;
  }

  //if we are updating the screen
  if(updatingScreen)
    beepScreen();

  return NONE;
}*/

CmdIhm ihm_newCmd(Key key,PotsInfo pot){
	/*
  static unsigned long timenew_ms=0; // only in the first time it gets the millis
  static unsigned long timeold_ms=0;
  //CmdIhm ret;
  bool debounce_ok;

  if(key!=KEY_NOTPRESSED){
    timenew_ms = millis();
  }

  debounce_ok  = ((timenew_ms-timeold_ms) > DEBOUNCEKEYS_MS);

  // debounce with time
  if(debounce_ok){
    timeold_ms = timenew_ms;
  }*/


   //txSerial.println(g_menu_state);
    switch(g_menu_state){
      case DEFAULT_SCREEN:
        return defaultScreenFunc(key, pot);
      break;
#ifdef USE_PRESSURE_SENSOR
      case CONFIG_SCREEN:
        return configScreenFunc(key);
      break;
      case GRAPH_SCREEN:
        return graphScreenFunc(key);
      break;
#endif //USE_PRESSURE_SENSOR
      default:
      break;
    } // switch(g_menu_state)

  return NONE;
}

PotsInfo ihm_checkPots(){
  static int oldPotVolume = 0, oldBPM = 0, oldIERatio;
  static int potCount = 0;
  static PotsInfo pots;
  oldPotVolume += (analogRead(POT_VOLUME));
  oldBPM += (analogRead(POT_BPM));
  oldIERatio += (analogRead(POT_IE_RATIO));
  potCount++;

  if(potCount == 16)
  {
  // division by 8 with shl 3
    pots.potvolume = oldPotVolume >> 4;
    pots.potbpm   = oldBPM >> 4;
    pots.potieratio = oldIERatio >> 4;
    oldPotVolume = 0;
    oldBPM = 0;
    oldIERatio = 0;
    potCount = 0;
  }
   return pots;
}




/*
void ihm_initScreen()
{

#ifndef LCD_128x64
	//Withou LCD for debug for debug proposes
	#if DEBUG_CODE != 0
		txSerial.println(F("Debug"));
		txSerial.println(F("FASTEN Vita Pneuma"));
		txSerial.println(F("Inicializando"));
		#ifndef START_LIMIT_SWITCH//if there is not a START_LIMIT_SWITCH, a warning message will be displayed
			  txSerial.println(F("Apenas Chave de Exp."));
		#endif
	#endif
	updatingScreen = 0;
#else

#if DEBUG_CODE != 0
	//lcd_128x64.drawStr(0, ((LCD_HEIGHT-FONT_HEIGHT_P)/2)-FONT_HEIGHT_P, "Modo Debug");
	lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*5.5f), (unsigned char)(((LCD_HEIGHT-FONT_HEIGHT_P)/2)-FONT_HEIGHT_P));
	lcd_128x64.print(F("Modo Debug"));
#endif


  lcd_128x64.firstPage();
	do {

	#if DEBUG_CODE != 0
		//lcd_128x64.drawStr(0, ((LCD_HEIGHT-FONT_HEIGHT_P)/2)-FONT_HEIGHT_P, "Modo Debug");
		lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*5.5f), (unsigned char)(((LCD_HEIGHT-FONT_HEIGHT_P)/2)-FONT_HEIGHT_P));
		lcd_128x64.print(F("Modo Debug"));
	#endif

	lcd_128x64.setFont(u8g_font_7x14Br);
	//lcd_128x64.drawStr(0, (LCD_HEIGHT-FONT_HEIGHT_P)/2, "FASTEN Vita Pneuma");
	lcd_128x64.setPrintPos((unsigned char)0, (unsigned char)((LCD_HEIGHT)/2));
	lcd_128x64.print(F("FASTEN Vita Pneuma"));
	lcd_128x64.setFont(FONT_TYPE);
	//lcd_128x64.drawStr(0, (LCD_HEIGHT+FONT_HEIGHT_P)/2, "Inicializando...");
	lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*2.5f), (unsigned char)(LCD_HEIGHT+FONT_HEIGHT_P*2)/2);
	lcd_128x64.print(F("Inicializando"));
	#ifndef START_LIMIT_SWITCH//if there is not a START_LIMIT_SWITCH, a warning message will be displayed
	  //lcd_128x64.drawStr(0, FONT_HEIGHT_P+(LCD_HEIGHT+FONT_HEIGHT_P)/2, "Apenas Chave de Exp.");
	  lcd_128x64.setPrintPos((unsigned char)(FONT_WIDTH*0.5f), (unsigned char)FONT_HEIGHT_P+(LCD_HEIGHT+FONT_HEIGHT_P*2)/2);
	  lcd_128x64.print(F("Apenas chave de exp."));
	#endif
	} while(lcd_128x64.nextPage());
	updatingScreen = 0;

#endif
}
*/

//TODO: maybe use inline to improve perfomance of the calls
CmdIhm ihm_check(){
  static unsigned long timenew_ms =0;
  static unsigned long timeold_ms =0;
  static Key iKey;
  static PotsInfo potsinfo;
  CmdIhm ret;

  timenew_ms = millis();
  // Avoid to call the function frequently to improve CPU performance
  if((timenew_ms - timeold_ms)>PERIODCHECKIHM_MS){
    iKey      = ihm_get_key();
    potsinfo  = ihm_checkPots();
    timeold_ms = timenew_ms;
    ret = ihm_newCmd(iKey, potsinfo);
    return ret;
  }
 return NONE;
}
