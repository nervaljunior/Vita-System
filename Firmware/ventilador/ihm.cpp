#include "ihm.h"
#include "global.h"

// include the library code for LCD 16x02
#include <LiquidCrystal.h>

#define DEBOUNCEKEYS_MS 500
#define TIMESCREEN_MS 4000
#define PERIODCHECKIHM_MS 50

#define VOLUMEMAX 100
#define VOLUMEMIN 10
#define VOLUMESTEP 10

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define ADCLEVES 1023

byte lcd_char_ambu_up[8] = {
  B10000,
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
};
byte lcd_char_ambu_down[8] = {
  B11111,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000,
  B10000,
};




static int adc_key_values[] =
{
          0,    // button RIGHT     0.00V - 0.39V
         80,    // button UP        0.39V - 1.46V
        205,    // button DOWN      1V    - 1.71V
        350,    // button LEFT      1.71V - 2.50V
        512,    // button SELECT    2.50V - 4.50V
        970     // button NOT_PRESS 4.74V -
};

//static StateIHM g_menu_state = MENUSELECIONEACONFIG;
StateIHM g_menu_state = MENUSELECIONEACONFIG;
static RespiratorInfo* gRespInfoPtr;

void ihm_init(RespiratorInfo* respinfoptr){
  lcd.createChar(0, lcd_char_ambu_up);
  lcd.createChar(1, lcd_char_ambu_down);
  lcd.begin(16, 2);
  gRespInfoPtr = respinfoptr;
}

// TODO: Check if the implemented debounce is enough
Key ihm_get_key() {

   int input = analogRead(A0);
   int k;
   // variables used for debounce
   static int k1,k2,k3;

   for ( k=1; k<6; k++) {
          if ( input < adc_key_values[k]) {
                  // Debounce of the analog port (avoid transitory)
                  if(k==k1 && k1==k2 && k2==k3){
                    return k;
                  }

                  k3 = k2;
                  k2 = k1;
                  k1 = k;
                  break;
          }
   }
   return 0;
 }

void printKey(Key key){
    // function to debug keypad
    switch(key){
      case (KEY_NOTPRESSED):Serial.println("KEY_NOTPRESSED");break;
      case (KEY_RT):Serial.println("KEY_RIGHT     ");break;
      case (KEY_UP):Serial.println("KEY_UP        ");break;
      case (KEY_DOWN):Serial.println("KEY_DOWN      ");break;
      case (KEY_LEFT):Serial.println("KEY_LEFT     ");break;
      case (KEY_SEL):Serial.println("KEY_SEL       ");break;
    }
}

CmdIhm menuselconfig(Key key){
  static int firstTime = 1;
  static int selected_cfg = 0;
  const int number_of_cfgs = 6;
  const int cursor_vec[number_of_cfgs] = {0, 3, 6, 9,12,15};

  if(firstTime){
    firstTime = 0;
    lcd.setCursor(0,0);
    lcd.print("Selecione a CFG:");
    lcd.setCursor(0,1);
    lcd.print("A  B  C  D  E  F");
    lcd.setCursor(0,1);
    lcd.blink();
  }else{
    if(KEY_LEFT == key){
      if(selected_cfg>0){
        --selected_cfg;
      }
    }

    if(KEY_RT == key){
      if(selected_cfg<(number_of_cfgs-1)){
        ++selected_cfg;
      }
    }
    lcd.setCursor(cursor_vec[selected_cfg],1);
    lcd.blink();
  }

  switch(selected_cfg){
	  case 0: return CFGA;
	  case 1: return CFGB;
	  case 2: return CFGC;
	  case 3: return CFGD;
	  case 4: return CFGE;
	  case 5: return CFGF;
	  default: return CFGA;
  }
}

void menutelapadrao(int bpm, float ie_ratio, int vol){
	// LCD Output
	/*****************
	*RPM  I:E    VOL.
	*30   1:4.0  100%
	*****************/
	char str[17];
	lcd.noBlink();
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("RPM  I:E    VOL.");
	lcd.setCursor(0,1);
	lcd.print(bpm);
	lcd.setCursor(5,1);
	lcd.print("1:");
	lcd.setCursor(7,1);
	dtostrf(ie_ratio,3,1,str);
	lcd.print(str);
	// check the number of digits are necessary to print the vol(1,2 or 3 digits)
	if(vol<10){
		lcd.setCursor(14,1);
	}else if(vol<100){
		lcd.setCursor(13,1);
	}else{
		lcd.setCursor(12,1);
	}
	lcd.print(vol,DEC);
	lcd.setCursor(15,1);
	lcd.print("%");
}

void menuChangedPot(int bpm_old, float ie_ratio_old, int vol_old, int  bpm_new, float ie_ratio_new, int vol_new){
	//TODO: optimize the code by removing sprintf

	// LCD Output:
	/*****************
	*10   1:2.1   50%	(new data)  
	*30   1:1.5  100%	(old data)
	*****************/
	char str[17];
	lcd.noBlink();
	lcd.clear();
	/*New data print*/
	lcd.setCursor(0,0);
	lcd.print(bpm_new);
	lcd.setCursor(5,0);
	lcd.print("1:");
	lcd.setCursor(7,0);
	dtostrf(ie_ratio_new,3,1,str);
	lcd.print(str);
	// check the number of digits are necessary to print the vol(1,2 or 3 digits)
	if(vol_new<10){
		lcd.setCursor(14,0);
	}else if(vol_new<100){
		lcd.setCursor(13,0);
	}else{
		lcd.setCursor(12,0);
	}
	lcd.print(vol_new,DEC);
	lcd.setCursor(15,0);
	lcd.print("%");
	



	/*Old data print*/
	lcd.setCursor(0,1);
	lcd.print(bpm_old);
	lcd.setCursor(5,1);
	lcd.print("1:");
	lcd.setCursor(7,1);
	dtostrf(ie_ratio_old,3,1,str);
	lcd.print(str);
	// check the number of digits are necessary to print the vol(1,2 or 3 digits)
	if(vol_old<10){
		lcd.setCursor(14,1);
	}else if(vol_old<100){
		lcd.setCursor(13,1);
	}else{
		lcd.setCursor(12,1);
	}
	lcd.print(vol_old,DEC);
	lcd.setCursor(15,1);
	lcd.print("%");
}

void menuPosChanged(int oldposE, int newposE){
	// LCD Output:
	/****************
	*N   |				(New config)
	*A        |			(Old config)
	*****************/

   // print old configuration
   lcd.setCursor(0,1);
   lcd.print("A               ");
   lcd.setCursor(oldposE,1);
   lcd.print("|");

   // print new configuration
   lcd.setCursor(0,0);
   lcd.print("N               ");
   lcd.setCursor(newposE,0);
   lcd.print("|");
   lcd.setCursor(newposE,0);
   lcd.blink();
}


/**
 * input pots
 * output bpm
 * output ie_ratio
 */
void convertPotsToVpmAndIeratio(PotsInfo pots, int* bpm, float* ie_ratio){
	// Get potentionmeter data and implment Hysteresis
	int idx;
	static bool firstExecutation =true;
	
	static unsigned short currentOutputLevelBpm,currentOutputLevelIeratio;	

	const short numberOfLevelsOutput = 21 ;
	
	// margin sets the 'stickyness' of the hysteresis or the relucatance to leave the current state.
	// It is measured in units of the the input level. As a guide it is a few percent of the
	// difference between two end points. Don't make the margin too wide or ranges may overlap.
	const unsigned short  margin = 15;   //  +/- 10
	
	// Both potentionmeter should be mapped to the same number of levels
	// Thus they can use the same endPointInput	
	const unsigned short endPointInput[numberOfLevelsOutput+1] = 
	{0,49,97,146,195,244,292,341,390,438,487,536,585,633,682,731,779,828,877,926,974,1023,};
	//{26,77,128,179,230,281,332,384,435,486,537,588,639,691,742,793,844,895,946,997};
	
	const float lut_ieratio[numberOfLevelsOutput] =
	{1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.6,2.7,2.8,2.9,3.0}; 

	const int lut_bpm[numberOfLevelsOutput]= 
	{10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
	
	unsigned short inputLevelBpm 		= pots.potbpm;
	unsigned short inputLevelIeratio 	= pots.potieratio;
	
	if(firstExecutation){
		firstExecutation=false;
		// The first time the code is executed is different
		// no hysteresis is possible because there is no "last state"
		for(idx = 1;idx<numberOfLevelsOutput-1;++idx){
			if(	inputLevelBpm>= endPointInput[idx] && 
				inputLevelBpm<=endPointInput[idx+1]){
				currentOutputLevelBpm = idx;
				break;
			}
		}
		
		for(idx = 1;idx<numberOfLevelsOutput-1;++idx){
			if(	inputLevelIeratio>= endPointInput[idx] && 
				inputLevelIeratio <=endPointInput[idx+1]){
				currentOutputLevelIeratio = idx;
				break;
			}
		}
		
		*bpm		= lut_bpm[currentOutputLevelBpm];
		*ie_ratio	= lut_ieratio[currentOutputLevelIeratio];
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
	


	// now test if input is between the outer margins for current output value
	if ( inputLevelBpm < lbBpm || inputLevelBpm > ubBpm ) {
	// determine new output level by scanning endPointInput array
		for ( idx = 0 ; idx < numberOfLevelsOutput ; idx++ ) {
			if (inputLevelBpm >= endPointInput[ idx ] && 
				inputLevelBpm <= endPointInput[ idx + 1 ] )
				break ;
		}
				currentOutputLevelBpm = idx;
#ifdef DEBUG_CODE
    Serial.print("inputLevelBpm:");
	Serial.print(inputLevelBpm);
	Serial.print("| idx: ");
	Serial.println(idx);
#endif
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
	
	
  
  *bpm 		= lut_bpm[currentOutputLevelBpm];
  *ie_ratio = lut_ieratio[currentOutputLevelIeratio];
}

CmdIhm ihm_newCmd(Key key,PotsInfo pot){
  static unsigned long timenew_ms=0; // only in the first time it gets the millis
  static unsigned long timeold_ms=0;
  static unsigned long timescreennew_ms=0; // time used for screen
  static unsigned long timescreenold_ms=0; // time used for screen

  bool debounce_ok;
  bool pot_changed;
	bool input_data_changed=false;

  int new_bpm;
  float new_ie_ratio;
  static int new_volume;
  static int old_bpm;
  static float old_ie_ratio;;

  static bool firstTime = true;
  static bool firstTimeOnMenuChanged = true;
  static bool firstTimeOnMenuKeyPressed = true;
  static bool firstTimeOnMenuTelaPadrao = true;
  static int changedPosI,changedPosE;

  if(key!=KEY_NOTPRESSED){
    timenew_ms = millis();
  }

  debounce_ok  = ((timenew_ms-timeold_ms) > DEBOUNCEKEYS_MS);

  // debounce with time
  if(debounce_ok){
    timeold_ms = timenew_ms;
  }

  convertPotsToVpmAndIeratio(pot, &new_bpm, &new_ie_ratio);
  
	pot_changed  = ((new_bpm != old_bpm) || (new_ie_ratio != old_ie_ratio));

	old_bpm = new_bpm;
	old_ie_ratio = new_ie_ratio;
	
	// KEY_UP and KEY_DOWN controls the volume, so if they were pressed, then
	//  the user provided a new input. pot_changed is similar
	input_data_changed = (key==KEY_UP || key==KEY_DOWN || pot_changed);
	


  	switch(g_menu_state){
  		case MENUSELECIONEACONFIG:
		  CmdIhm selCfg;
          if(debounce_ok || firstTime){
            firstTime = 0;
            selCfg = menuselconfig(key);
            if(KEY_SEL == key){
              lcd.noBlink();
              g_menu_state = TELAPADRAO;
			        return selCfg;
            } else {
              g_menu_state = MENUSELECIONEACONFIG;
            }
			switch(key){
				case(KEY_UP): return CMD_KEY_UP;
				case(KEY_LEFT): return CMD_KEY_LEFT;
				case(KEY_DOWN): return CMD_KEY_DOWN;
				case(KEY_RT): return CMD_KEY_RT;
				default : return NONE;
			}
          }
        // TODO: get selected configuration from memory
  		break; //MENUSELECIONEACONFIG

      case TELAPADRAO:
	    if(firstTimeOnMenuTelaPadrao){
			menutelapadrao(gRespInfoPtr->bpm,gRespInfoPtr->ie_ratio,gRespInfoPtr->volume);
			firstTimeOnMenuTelaPadrao = false;
		}
        g_menu_state = TELAPADRAO;
        if(input_data_changed){
          g_menu_state = MENUINPUTCHANGED;
		  firstTimeOnMenuTelaPadrao = true;
		  return NONE;
        }
        if(debounce_ok && (KEY_RT== key || KEY_LEFT==key)){
          g_menu_state = MENUKEYPRESSED;
		  firstTimeOnMenuTelaPadrao = true;
		  return NONE;
        }
		if(debounce_ok && KEY_SEL== key){
		  return SELECT;
		}
		return NONE;
      break; //TELAPADRAO

      case MENUINPUTCHANGED:
        if(firstTimeOnMenuChanged){
			new_volume = gRespInfoPtr->volume;
		}
		
		if(firstTimeOnMenuChanged || input_data_changed){
          // Start counting time
          timescreenold_ms = millis();
          firstTimeOnMenuChanged = false;
          g_menu_state = MENUINPUTCHANGED;
        }

        timescreennew_ms = millis();
        if((timescreennew_ms- timescreenold_ms)>TIMESCREEN_MS){
            g_menu_state = TELAPADRAO;
            // we are leaving this state, so the next time it enters here
            // it will  be the first time on this menu
            firstTimeOnMenuChanged = true;
        }else{
			if(KEY_UP==key && debounce_ok){
				new_volume+=VOLUMESTEP;
				if(new_volume>VOLUMEMAX){
					new_volume = VOLUMEMAX;
				}
				input_data_changed = true;
			}
			if(KEY_DOWN==key && debounce_ok){
				new_volume-=VOLUMESTEP;
				if(new_volume<VOLUMEMIN){
					new_volume = VOLUMEMIN;
				}
				input_data_changed = true;
			}
			
            if(input_data_changed){
				menuChangedPot(gRespInfoPtr->bpm,gRespInfoPtr->ie_ratio,gRespInfoPtr->volume,new_bpm,new_ie_ratio,new_volume);
			}
            if(KEY_SEL==key){
                gRespInfoPtr->bpm 		= new_bpm;
                gRespInfoPtr->ie_ratio 	= new_ie_ratio;
				gRespInfoPtr->volume	= new_volume;
				menuChangedPot(gRespInfoPtr->bpm,gRespInfoPtr->ie_ratio,gRespInfoPtr->volume,new_bpm,new_ie_ratio,new_volume);
                //TODO: save data on EEPROM
                // give more time to user see the changes in the current screen
                timescreenold_ms = millis();
                g_menu_state =  MENUINPUTCHANGED;
				return NEWDATA;
            }
        }
		return NONE;
      break; // MENUINPUTCHANGED

      case MENUKEYPRESSED:

        if(firstTimeOnMenuKeyPressed){
            // These assigments should be done *only* in the first time this state is entered
            //changedPosI = gRespInfoPtr->pos_ins;
            changedPosE = gRespInfoPtr->pos_exp;
        }

        if(firstTimeOnMenuKeyPressed || key!=KEY_NOTPRESSED){
          // Start counting time
          timescreenold_ms = millis();
          firstTimeOnMenuKeyPressed = false;
          g_menu_state = MENUKEYPRESSED;
        }

        timescreennew_ms = millis();
        if((timescreennew_ms- timescreenold_ms)>TIMESCREEN_MS){
            g_menu_state = TELAPADRAO;
            // we are leaving this state, so the next it enters here
            // it will  be the first time on this menu
            firstTimeOnMenuKeyPressed = true;
        }else{
            if(debounce_ok){
              switch(key){
                // KEY_RT and KEY_LEFT increase and decrease PosE, respectively
                // changedPosI should be lower than changedPosE
                case KEY_RT:    if(changedPosE<POSMAX){++changedPosE;}break;
                case KEY_LEFT:  if(changedPosE>POSMIN+1 && changedPosE>changedPosI+1){--changedPosE;}break;
                default : break;
              }
            }//if(debounce_ok)



            //printKey(key);
            if(KEY_NOTPRESSED!=key){
              //menuKeyPressd(key,gRespInfoPtr->pos_ins,gRespInfoPtr->pos_exp, changedPosI,changedPosE);
			  menuPosChanged(gRespInfoPtr->pos_exp,changedPosE);
            }

            if(KEY_SEL==key){
              //gRespInfoPtr->pos_ins = changedPosI;
              gRespInfoPtr->pos_exp = changedPosE;
			  menuPosChanged(gRespInfoPtr->pos_exp,changedPosE);
              g_menu_state = MENUKEYPRESSED;
			  return NEWDATA;
            }

        }

      break; // MENUKEYPRESSED

      default: break;
  	} // switch(g_menu_state)

	return NONE;
}

PotsInfo ihm_checkPots(){
  static int oldPot2 = 0, oldPot1 = 0;
  static int potCount = 0;
  static PotsInfo pots;
  oldPot1 += (analogRead(pot1));
  oldPot2 += (analogRead(pot2));
  potCount++;
  
  if(potCount == 4)
  {
	// division by 4 with shl 2
    pots.potbpm 	= oldPot1 >> 2;
    pots.potieratio	= oldPot2 >> 2;
    oldPot1 = 0;
    oldPot2 = 0;
    potCount = 0;
  }
   return pots;
}


void ihm_initScreen(){
#if 1
   lcd.setCursor(0,0);
   lcd.print("Respirador INESC");
   lcd.setCursor(0,1);
   lcd.print("Inicializando...");

#else
   // With Animation
   static unsigned long timeAnimNew_ms=0;
   static unsigned long timeAnimOld_ms=0;
   static int cntForAnim = 0;

   timeAnimNew_ms = millis();

	if((timeAnimNew_ms-timeAnimOld_ms)>500){

		// reset time
		timeAnimOld_ms = timeAnimNew_ms;
		// Increment points for animation
		++cntForAnim;

		lcd.setCursor(0,0);
		lcd.print("Respirador INESC");
		lcd.setCursor(0,1);
		lcd.print("Inicializando   ");

		// Set the cursor where the first point should be positioned
		lcd.setCursor(13,1);

		switch(cntForAnim){
			case 1: lcd.print(".");break;
			case 2: lcd.print("..");break;
			case 3: lcd.print("...");break;
			default: cntForAnim=0; break;
		}
	}
#endif
}

//TODO: maybe use inline to improve perfomance of the calls
CmdIhm ihm_check(){
	static unsigned long timenew_ms =0;
	static unsigned long timeold_ms =0;
	static Key iKey;
	static PotsInfo potsinfo;
	CmdIhm ret;

	timenew_ms = millis();
	// Avoid to call the funcation frequently to improve CPU performance
	if((timenew_ms - timeold_ms)>PERIODCHECKIHM_MS){
		iKey      = ihm_get_key();
		potsinfo  = ihm_checkPots();
		timeold_ms = timenew_ms;
		ret = ihm_newCmd(iKey, potsinfo);
		return ret;
	}
 return NONE;
}
