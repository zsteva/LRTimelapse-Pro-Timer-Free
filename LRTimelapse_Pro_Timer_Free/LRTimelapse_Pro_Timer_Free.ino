#include <LiquidCrystal.h>


/*
  Pro-Timer Free
  Gunther Wegner
  http://gwegner.de
  http://lrtimelapse.com
*/

#include <LiquidCrystal.h>
#include "LCD_Keypad_Reader.h"			// credits to: http://www.hellonull.com/?p=282

const String CAPTION = "Pro-Timer 0.85";

LCD_Keypad_Reader keypad;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);	//Pin assignments for SainSmart LCD Keypad Shield

const int CAMERA_PIN = 12;
const int LIGHT_PIN = 11;
const int FLASH_PIN = 13;

const int NONE = 0;						// Key constants
const int SELECT = 1;
const int LEFT = 2;
const int UP = 3;
const int DOWN = 4;
const int RIGHT = 5;

const int BACK_LIGHT = 10;

const int RELEASE_TIME = 100;			// Shutter release time for camera

const int keyRepeatRate = 100;			// when held, key repeats 1000 / keyRepeatRate times per second
const int keySampleRate = 50;			// ms between checking keypad for key


int localKey = 0;						// The current pressed key
int lastKeyPressed = -1;				// The last pressed key

unsigned long lastKeyCheckTime = 0;
unsigned long lastKeyPressTime = 0;

int sameKeyCount = 0;
unsigned long previousMillis = 0;		// Timestamp of last shutter release
unsigned long previousMillis2 = 0;

float interval = 60.0;					// the current interval
long maxNoOfShots = 0;
int isRunning = 0;						// flag indicates intervalometer is running

int imageCount = 0;                   	// Image count since start of intervalometer

unsigned long rampDuration = 10;		// ramping duration
float rampTo = 0.0;						// ramping interval
unsigned long rampingStartTime = 0;		// ramping start time
unsigned long rampingEndTime = 0;		// ramping end time
float intervalBeforeRamping = 0;		// interval before ramping

unsigned long runningTime = 43200000; // 12h
unsigned long lightOnTime = 79200000; // 22h
unsigned long lightOffTime = 7200000; // 2h
unsigned long lightIntervalTime = 86400000; // 24h

/* DEV
unsigned long runningTime = 0;
unsigned long lightOnTime = 60000;
unsigned long lightOffTime = 60000;
unsigned long lightIntervalTime = 120000;

 */

int light1On = 1;
int light1Off = 0;
int light2On = 0;

unsigned long preLight = 5000;
unsigned long postLight = 5000;


boolean backLight = HIGH;				// The current settings for the backlight

const int SCR_INTERVAL = 0;				// menu workflow constants
const int SCR_CHTIME = 21;
const int SCR_CHLIGHTON_TIME = 22;
const int SCR_CHLIGHTOFF_TIME = 23;
const int SCR_SHOOTS = 1;
const int SCR_RUNNING = 2;
const int SCR_CONFIRM_END = 3;
const int SCR_SETTINGS = 4;
const int SCR_PAUSE = 5;
const int SCR_RAMP_TIME = 6;
const int SCR_RAMP_TO = 7;
const int SCR_DONE = 8;


int currentMenu = 0;					// the currently selected menu
int settingsSel = 1;					// the currently selected settings option

/**
   Initialize everything
*/
void setup() {

  pinMode(BACK_LIGHT, OUTPUT);
  digitalWrite(BACK_LIGHT, HIGH);		// Turn backlight on.

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);

  // print welcome screen))
  lcd.print("LRTimelapse.com");
  lcd.setCursor(0, 1);
  lcd.print( CAPTION );

  pinMode(CAMERA_PIN, OUTPUT);					// initialize output pin for camera release
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FLASH_PIN, OUTPUT);

  delay(2000);							// wait a moment...

  Serial.begin(9600);
  Serial.println( CAPTION );

  lcd.clear();
}

/**
   The main loop
*/
void loop() {

  if (millis() > lastKeyCheckTime + keySampleRate) {
    lastKeyCheckTime = millis();
    localKey = keypad.getKey();

    if (localKey != lastKeyPressed) {
      processKey();
    } else {
      // key value has not changed, key is being held down, has it been long enough?
      // (but don't process localKey = 0 = no key pressed)
      if (localKey != 0 && millis() > lastKeyPressTime + keyRepeatRate) {
        // yes, repeat this key
        if ( (localKey == UP ) || ( localKey == DOWN ) ) {
          processKey();
        }
      }
    }

    if ( currentMenu == SCR_RUNNING ) {
      printScreen();	// update running screen in any case
    }
  }
  if ( isRunning ) {	// release camera, do ramping if running
    running();
  }

}

void actionInterval() {
          currentMenu = SCR_INTERVAL;
        isRunning = 0;
        lcd.clear();

}

int changeTimePos = 0;

void actionChangeTime() {
  changeTimePos = 0;
  currentMenu = SCR_CHLIGHTON_TIME;
  isRunning = 0;
  lcd.clear();
}

void actionRunning() {
        currentMenu = SCR_RUNNING;
        previousMillis = millis();
        previousMillis2 = millis();
        // runningTime = 0;
        isRunning = 1;

        lcd.noCursor();
        lcd.clear();


        // do the first release instantly, the subsequent ones will happen in the loop
        //releaseCamera();
        //imageCount++;
  
}

/**
   Process the key presses - do the Menu Navigation
*/
void processKey() {
  long timeTmp, timeTmp2;
  
  lastKeyPressed = localKey;
  lastKeyPressTime = millis();

  // select key will switch backlight at any time
  if ( localKey == SELECT ) {
    if ( backLight == HIGH ) {
      backLight = LOW;
    } else {
      backLight = HIGH;
    }
    digitalWrite(BACK_LIGHT, backLight); // Turn backlight on.
  }

  // do the menu navigation
  switch ( currentMenu ) {

    case SCR_INTERVAL:

      if ( localKey == UP ) {
        // interval = (float)((int)(interval * 10) + 1) / 10; // round to 1 decimal place
        interval = (float)((int)(interval * 1) + 1) / 1; // round to 1 decimal place
        if (interval > 900) {
          interval = 900;
        }
        
        // if ( interval > 99 ) { // no intervals longer as 99secs - those would scramble the display
        //   interval = 99;
        // }
      }

      if ( localKey == DOWN ) {
        if ( interval > 12) {
          // interval = (float)((int)(interval * 10) - 1) / 10; // round to 1 decimal place
          interval = (float)((int)(interval * 1) - 1) / 1;
        }
      }

/*      if ( localKey == RIGHT ) {
        lcd.clear();
        rampTo = interval;			// set rampTo default to the current interval
        currentMenu = SCR_SHOOTS;
      }
*/

      if (localKey == RIGHT) {
        actionChangeTime();
      }
      break;
      
    case SCR_CHTIME:
    case SCR_CHLIGHTON_TIME:
    case SCR_CHLIGHTOFF_TIME:

      if (localKey == UP) {
        if (currentMenu == SCR_CHLIGHTON_TIME) {
           timeTmp = lightOnTime;
        } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
          timeTmp = lightOffTime;
        } else {
          timeTmp = runningTime;
        }
        
        if (changeTimePos == 0)      { timeTmp += 36000000; }
        else if (changeTimePos == 1) { timeTmp += 3600000; }
        //
        else if (changeTimePos == 3) { timeTmp += 600000; }
        else if (changeTimePos == 4) { timeTmp += 60000; }
        
        else if (changeTimePos == 6) { timeTmp += 10000; }
        else if (changeTimePos == 7) { timeTmp += 1000; }


        if (currentMenu == SCR_CHLIGHTON_TIME || currentMenu == SCR_CHLIGHTOFF_TIME) {
          while (timeTmp < 0) {
            timeTmp += 86400000;
          }
          timeTmp %= 86400000;
        } else {
          while (timeTmp < 0 && lightIntervalTime > 0) {
            timeTmp += lightIntervalTime;
          }
          timeTmp %= lightIntervalTime;          
        }

        if (currentMenu == SCR_CHLIGHTON_TIME) {
          lightOnTime = timeTmp;
          lightIntervalTime = lightOnTime + lightOffTime;
        } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
          lightOffTime = timeTmp;
          lightIntervalTime = lightOnTime + lightOffTime;
        } else {
          runningTime = timeTmp;
        }
        
      }

      
      if (localKey == DOWN) {
        
        if (currentMenu == SCR_CHLIGHTON_TIME) {
           timeTmp = lightOnTime;
        } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
          timeTmp = lightOffTime;
        } else {
          timeTmp = runningTime;
        }
 
        if (changeTimePos == 0)      { timeTmp -= 36000000; }
        else if (changeTimePos == 1) { timeTmp -= 3600000; }
        //
        else if (changeTimePos == 3) { timeTmp -= 600000; }
        else if (changeTimePos == 4) { timeTmp -= 60000; }
        
        else if (changeTimePos == 6) { timeTmp -= 10000; }
        else if (changeTimePos == 7) { timeTmp -= 1000; }


        if (currentMenu == SCR_CHLIGHTON_TIME || currentMenu == SCR_CHLIGHTOFF_TIME) {
          while (timeTmp < 0) {
            timeTmp += 86400000;
          }
          timeTmp %= 86400000;
        } else {
          while (timeTmp < 0 && lightIntervalTime > 0) {
            timeTmp += lightIntervalTime;
          }
          timeTmp %= lightIntervalTime;          
        }
        
        if (currentMenu == SCR_CHLIGHTON_TIME) {
          lightOnTime = timeTmp;
          lightIntervalTime = lightOnTime + lightOffTime;
        } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
          lightOffTime = timeTmp;
          lightIntervalTime = lightOnTime + lightOffTime;
        } else {
          runningTime = timeTmp;
        }
        
      }
      if ( localKey == LEFT ) {
        if (changeTimePos == 7) { changeTimePos = 6; }
        else if (changeTimePos == 6) { changeTimePos = 4; }
        else if (changeTimePos == 4) { changeTimePos = 3; }
        else if (changeTimePos == 3) { changeTimePos = 1; }
        else if (changeTimePos > 0) {
          changeTimePos--;
        } else {
  
          if (currentMenu == SCR_CHLIGHTON_TIME) {
            actionInterval();        
          } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
            currentMenu = SCR_CHLIGHTON_TIME;
            lcd.clear();
          } else {
            currentMenu = SCR_CHLIGHTOFF_TIME;
            lcd.clear();
          }

        }
      }
  
      if ( localKey == RIGHT ) { // Start shooting
        if (changeTimePos == 7) {
          if (currentMenu == SCR_CHLIGHTON_TIME) {
            currentMenu = SCR_CHLIGHTOFF_TIME;
            lcd.clear();
          } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
            currentMenu = SCR_CHTIME;
            lcd.clear();
          } else {
            actionRunning();
          }
        } else if (changeTimePos == 6) { changeTimePos = 7; }
        else if (changeTimePos == 4) { changeTimePos = 6; }
        else if (changeTimePos == 3) { changeTimePos = 4; }
        else if (changeTimePos == 1) { changeTimePos = 3; }
        else {
          changeTimePos++;
        }
      }
      
      break;

    case SCR_SHOOTS:

      if ( localKey == UP ) {
        if ( maxNoOfShots >= 2500 ) {
          maxNoOfShots += 100;
        } else if ( maxNoOfShots >= 1000 ) {
          maxNoOfShots += 50;
        } else if ( maxNoOfShots >= 100 ) {
          maxNoOfShots += 25;
        } else if ( maxNoOfShots >= 10 ) {
          maxNoOfShots += 10;
        } else {
          maxNoOfShots ++;
        }
        if ( maxNoOfShots >= 9999 ) { // prevents screwing the ui
          maxNoOfShots = 9999;
        }
      }

      if ( localKey == DOWN ) {
        if ( maxNoOfShots > 2500 ) {
          maxNoOfShots -= 100;
        } else if ( maxNoOfShots > 1000 ) {
          maxNoOfShots -= 50;
        } else if ( maxNoOfShots > 100 ) {
          maxNoOfShots -= 25;
        } else if ( maxNoOfShots > 10 ) {
          maxNoOfShots -= 10;
        } else if ( maxNoOfShots > 0) {
          maxNoOfShots -= 1;
        } else {
          maxNoOfShots = 0;
        }
      }

      if ( localKey == LEFT ) {
        actionChangeTime();
        
      }

      if ( localKey == RIGHT ) { // Start shooting
        actionRunning();
      }
      break;

    case SCR_RUNNING:

      if ( localKey == LEFT ) { // LEFT from Running Screen aborts

        if ( rampingEndTime == 0 ) { 	// if ramping not active, stop the whole shot, other otherwise only the ramping
          if ( isRunning ) { 		// if is still runing, show confirm dialog
            currentMenu = SCR_CONFIRM_END;
          } else {				// if finished, go to start screen
            currentMenu = SCR_INTERVAL;
          }
          lcd.clear();
        } else { // stop ramping
          rampingStartTime = 0;
          rampingEndTime = 0;
        }

      }

      if ( localKey == RIGHT ) {
        currentMenu = SCR_SETTINGS;
        lcd.clear();
      }
      break;

    case SCR_CONFIRM_END:
      if ( localKey == LEFT ) { // Really abort
        currentMenu = SCR_INTERVAL;
        isRunning = 0;
//        imageCount = 0;
//        runningTime = 0;
        lcd.clear();
      }
      if ( localKey == RIGHT ) { // resume
        currentMenu = SCR_RUNNING;
        lcd.clear();
      }
      break;

    case SCR_SETTINGS:

      if ( localKey == DOWN && settingsSel == 1 ) {
        settingsSel = 2;
      }

      if ( localKey == UP && settingsSel == 2 ) {
        settingsSel = 1;
      }

      if ( localKey == LEFT ) {
        settingsSel = 1;
        currentMenu = SCR_RUNNING;
        lcd.clear();
      }

      if ( localKey == RIGHT && settingsSel == 1 ) {
        isRunning = 0;
        currentMenu = SCR_PAUSE;
        lcd.clear();
      }

      if ( localKey == RIGHT && settingsSel == 2 ) {
        currentMenu = SCR_RAMP_TIME;
        lcd.clear();
      }
      break;

    case SCR_PAUSE:

      if ( localKey == LEFT ) {
        currentMenu = SCR_RUNNING;
        isRunning = 1;
        previousMillis = millis() - (imageCount * 1000); // prevent counting the paused time as running time;
        lcd.clear();
      }
      break;

    case SCR_RAMP_TIME:

      if ( localKey == RIGHT ) {
        currentMenu = SCR_RAMP_TO;
        lcd.clear();
      }

      if ( localKey == LEFT ) {
        currentMenu = SCR_SETTINGS;
        settingsSel = 2;
        lcd.clear();
      }

      if ( localKey == UP ) {
        if ( rampDuration >= 10) {
          rampDuration += 10;
        } else {
          rampDuration += 1;
        }
      }

      if ( localKey == DOWN ) {
        if ( rampDuration > 10 ) {
          rampDuration -= 10;
        } else {
          rampDuration -= 1;
        }
        if ( rampDuration <= 1 ) {
          rampDuration = 1;
        }
      }
      break;

    case SCR_RAMP_TO:

      if ( localKey == LEFT ) {
        currentMenu = SCR_RAMP_TIME;
        lcd.clear();
      }

      if ( localKey == UP ) {
        rampTo = (float)((int)(rampTo * 10) + 1) / 10; // round to 1 decimal place
        if ( rampTo > 99 ) { // no intervals longer as 99secs - those would scramble the display
          rampTo = 99;
        }
      }

      if ( localKey == DOWN ) {
        if ( rampTo > 0.2) {
          rampTo = (float)((int)(rampTo * 10) - 1) / 10; // round to 1 decimal place
        }
      }

      if ( localKey == RIGHT ) { // start Interval ramping
        if ( rampTo != interval ) { // only if a different Ramping To interval has been set!
          intervalBeforeRamping = interval;
          rampingStartTime = millis();
          rampingEndTime = rampingStartTime + rampDuration * 60 * 1000;
        }

        // go back to main screen
        currentMenu = SCR_RUNNING;
        lcd.clear();
      }
      break;

    case SCR_DONE:

      if ( localKey == LEFT || localKey == RIGHT ) {
        currentMenu = SCR_INTERVAL;
        isRunning = 0;
//        imageCount = 0;
//        runningTime = 0;
        lcd.clear();
      }
      break;
  }
  printScreen();
}

void printScreen() {

  switch ( currentMenu ) {

    case SCR_INTERVAL:
      printIntervalMenu();
      break;

    case SCR_SHOOTS:
      printNoOfShotsMenu();
      break;

    case SCR_CHTIME:
    case SCR_CHLIGHTON_TIME:
    case SCR_CHLIGHTOFF_TIME:
      printTimeScreen();
      break;

    case SCR_RUNNING:
      printRunningScreen();
      break;

    case SCR_CONFIRM_END:
      printConfirmEndScreen();
      break;

    case SCR_SETTINGS:
      printSettingsMenu();
      break;

    case SCR_PAUSE:
      printPauseMenu();
      break;

    case SCR_RAMP_TIME:
      printRampDurationMenu();
      break;

    case SCR_RAMP_TO:
      printRampToMenu();
      break;

    case SCR_DONE:
      printDoneScreen();
      break;
  }
}

// ........... [light off] 5sec [release] 5sec [light on]...................

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;
int releaseOnce = 0;

unsigned long updateLight = 0;
/**
   Running, releasing Camera
*/
void running() {
  unsigned long curr_millis = millis();

  if (time1 == 0) {
    time2 = curr_millis + interval * 1000;
    time1 = time2 - preLight;
    time3 = time2 + postLight; 

    light1Off = 0;
    light2On = 0;
    releaseOnce = 0;
    digitalWrite(LIGHT_PIN, light1On && !light1Off ? HIGH : LOW);
    digitalWrite(FLASH_PIN, light2On ? HIGH : LOW);
  }

  if (time3 < curr_millis) {
    light1Off = 0;
    light2On = 0;
    digitalWrite(LIGHT_PIN, light1On && !light1Off ? HIGH : LOW);
    digitalWrite(FLASH_PIN, light2On ? HIGH : LOW);

    time2 = time2 + interval * 1000;
    time1 = time2 - preLight;
    time3 = time2 + postLight; 
    releaseOnce = 0;
  }

  if (time2 < curr_millis && releaseOnce == 0) {
    releaseCamera();
    imageCount++;
    releaseOnce = 1;
  }

  if (time1 < curr_millis) {
    light1Off = 1;
    light2On = 1;
    digitalWrite(LIGHT_PIN, light1On && !light1Off ? HIGH : LOW);
    digitalWrite(FLASH_PIN, light2On ? HIGH : LOW);
  }

  // do this every interval only
  // if ( ( millis() - previousMillis ) >=  ( ( interval * 1000 )) ) {

/*
    if ( ( maxNoOfShots != 0 ) && ( imageCount >= maxNoOfShots ) ) { // sequence is finished
      // stop shooting
      isRunning = 0;
      currentMenu = SCR_DONE;
      lcd.clear();
      printDoneScreen(); // invoke manually
    } else { 
  */    
      // is running
      // runningTime += (millis() - previousMillis );
 //     previousMillis = millis();
 //     releaseCamera();
//      imageCount++;
    
    //}
 /// }

  if (updateLight < curr_millis) {
    runningTime += (curr_millis - previousMillis2);
    previousMillis2 = curr_millis;

    if (runningTime > lightIntervalTime) {
      runningTime %= lightIntervalTime;
    }

    if (runningTime < lightOnTime) {
      light1On = 1;
    } else {
      light1On = 0;
    }

    digitalWrite(LIGHT_PIN, light1On && !light1Off ? HIGH : LOW);

    updateLight = curr_millis + 1000;
  }


  // do this always (multiple times per interval)
  // possiblyRampInterval();
}

/**
   If ramping was enabled do the ramping
*/
void possiblyRampInterval() {
/*
  if ( ( millis() < rampingEndTime ) && ( millis() >= rampingStartTime ) ) {
    interval = intervalBeforeRamping + ( (float)( millis() - rampingStartTime ) / (float)( rampingEndTime - rampingStartTime ) * ( rampTo - intervalBeforeRamping ) );
  } else {
    rampingStartTime = 0;
    rampingEndTime = 0;
  }
*/
}

/**
   Actually release the camera
*/
void releaseCamera() {

  lcd.setCursor(15, 0);
  lcd.print((char)255);

  digitalWrite(CAMERA_PIN, HIGH);
  delay(RELEASE_TIME);
  digitalWrite(CAMERA_PIN, LOW);

  lcd.setCursor(15, 0);
  lcd.print(" ");
}

/**
   Pause Mode
*/
void printPauseMenu() {

  lcd.setCursor(0, 0);
  lcd.print("PAUSE...        ");
  lcd.setCursor(0, 1);
  lcd.print("< Continue");
}

// ---------------------- SCREENS AND MENUS -------------------------------

/**
   Configure Interval setting (main screen)
*/
void printIntervalMenu() {

  lcd.setCursor(0, 0);
  lcd.print("Interval");
  lcd.setCursor(0, 1);
  lcd.print( interval );
  lcd.print( "     " );
}

/**
   Configure no of shots - 0 means infinity
*/
void printNoOfShotsMenu() {

  lcd.setCursor(0, 0);
  lcd.print("No of shots");
  lcd.setCursor(0, 1);
  if ( maxNoOfShots > 0 ) {
    lcd.print( printInt( maxNoOfShots, 4 ) );
  } else {
    lcd.print( "----" );
  }
}

/**
   Print running screen
*/
void printRunningScreen() {
  
  lcd.setCursor(0, 0);
  lcd.print( printInt( imageCount, 4 ) );

  if ( maxNoOfShots > 0 ) {
    lcd.print( " R:" );
    lcd.print( printInt( maxNoOfShots - imageCount, 4 ) );
    lcd.print( " " );

    lcd.setCursor(0, 1);
    // print remaining time
    unsigned long remainingSecs = (maxNoOfShots - imageCount) * interval;
    lcd.print( "T-");
    lcd.print( fillZero( remainingSecs / 60 / 60 ) );
    lcd.print( ":" );
    lcd.print( fillZero( ( remainingSecs / 60 ) % 60 ) );
  }

  updateTime();

  lcd.setCursor(0, 1);

  if (light1On && !light1Off) {
    lcd.print("L");
  } else {
    lcd.print(" ");
  }


  lcd.setCursor(1, 1);

  if (light2On) {
    lcd.print("f");
  } else {
    lcd.print(" ");
  }
  
  

  lcd.setCursor(11, 0);
  if ( millis() < rampingEndTime ) {
    lcd.print( "*" );
  } else {
    lcd.print( " " );
  }
  lcd.print( printFloat( interval, 4, 1 ) );
}

void printTimeScreen() {

  unsigned long finerRunningTime;
  String msg;
  
       if (currentMenu == SCR_CHLIGHTON_TIME) {
           finerRunningTime = lightOnTime;
            msg = "Light On Time";
        } else if (currentMenu == SCR_CHLIGHTOFF_TIME) {
          finerRunningTime = lightOffTime;
          msg = "Light Off Time";
        } else {
          finerRunningTime = runningTime;
          msg = "Current Time";
        }

  // Serial.println(finerRunningTime);
 
    int hours = finerRunningTime / 1000 / 60 / 60;
    int minutes = (finerRunningTime / 1000 / 60) % 60;
    int secs = (finerRunningTime / 1000 ) % 60;

    String sHours = fillZero( hours );
    String sMinutes = fillZero( minutes );
    String sSecs = fillZero( secs );

    lcd.setCursor(0, 0);
    lcd.print(msg);

    lcd.setCursor(8, 1);
    lcd.print( sHours );
    lcd.setCursor(10, 1);
    lcd.print(":");
    lcd.setCursor(11, 1);
    lcd.print( sMinutes );
    lcd.setCursor(13, 1);
    lcd.print(":");
    lcd.setCursor(14, 1);
    lcd.print( sSecs );

    lcd.setCursor(8 + changeTimePos, 1);
    lcd.cursor();
}

void printDoneScreen() {

  // print elapsed image count))
  lcd.setCursor(0, 0);
  lcd.print("Done ");
  lcd.print( imageCount );
  lcd.print( " shots.");

  // print elapsed time when done
  lcd.setCursor(0, 1);
  lcd.print( "t=");
  lcd.print( fillZero( runningTime / 1000 / 60 / 60 ) );
  lcd.print( ":" );
  lcd.print( fillZero( ( runningTime / 1000 / 60 ) % 60 ) );
  lcd.print("    ;-)");
}

void printConfirmEndScreen() {
  lcd.setCursor(0, 0);
  lcd.print( "Stop shooting?");
  lcd.setCursor(0, 1);
  lcd.print( "< Stop    Cont.>");
}

/**
   Update the time display in the main screen
*/
void updateTime() {

  unsigned long finerRunningTime = runningTime; // + (millis() - previousMillis);

  if ( isRunning ) {

    int hours = finerRunningTime / 1000 / 60 / 60;
    int minutes = (finerRunningTime / 1000 / 60) % 60;
    int secs = (finerRunningTime / 1000 ) % 60;

    String sHours = fillZero( hours );
    String sMinutes = fillZero( minutes );
    String sSecs = fillZero( secs );

    lcd.setCursor(8, 1);
    lcd.print( sHours );
    lcd.setCursor(10, 1);
    lcd.print(":");
    lcd.setCursor(11, 1);
    lcd.print( sMinutes );
    lcd.setCursor(13, 1);
    lcd.print(":");
    lcd.setCursor(14, 1);
    lcd.print( sSecs );
  } else {
    lcd.setCursor(8, 1);
    lcd.print("   Done!");
  }
}

/**
   Print Settings Menu
*/
void printSettingsMenu() {

  lcd.setCursor(1, 0);
  lcd.print("Pause          ");
  lcd.setCursor(1, 1);
  lcd.print("Ramp Interval  ");
  lcd.setCursor(0, settingsSel - 1);
  lcd.print(">");
  lcd.setCursor(0, 1 - (settingsSel - 1));
  lcd.print(" ");
}

/**
   Print Ramping Duration Menu
*/
void printRampDurationMenu() {

  lcd.setCursor(0, 0);
  lcd.print("Ramp Time (min) ");

  lcd.setCursor(0, 1);
  lcd.print( rampDuration );
  lcd.print( "     " );
}

/**
   Print Ramping To Menu
*/
void printRampToMenu() {

  lcd.setCursor(0, 0);
  lcd.print("Ramp to (Intvl)");

  lcd.setCursor(0, 1);
  lcd.print( rampTo );
  lcd.print( "     " );
}




// ----------- HELPER METHODS -------------------------------------

/**
   Fill in leading zero to numbers in order to always have 2 digits
*/
String fillZero( int input ) {

  String sInput = String( input );
  if ( sInput.length() < 2 ) {
    sInput = "0";
    sInput.concat( String( input ));
  }
  return sInput;
}

String fillZero5( int input ) {

  String sInput = String( input );
  if ( sInput.length() == 1 ) {
    sInput = "0000";
  }
  else if (sInput.length() == 2) {
    sInput = "000";
  }
  else if (sInput.length() == 3) {
    sInput = "00";
  }
  else if (sInput.length() == 4) {
    sInput = "0";
  }
  else {
    sInput = "";
  }

  sInput.concat( String( input ));

  return sInput;
}


String printFloat(float f, int total, int dec) {

  static char dtostrfbuffer[8];
  String s = dtostrf(f, total, dec, dtostrfbuffer);
  return s;
}

String printInt( int i, int total) {
  float f = i;
  static char dtostrfbuffer[8];
  String s = dtostrf(f, total, 0, dtostrfbuffer);
  return s;
}

