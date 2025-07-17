#include <LiquidCrystal.h>    // Library for 16x2 LCD display

// === pin definitions ===
// output pins
const int pumpInPin = 2;     // represents submerible pump 
const int pumpOutPin = 3;    // represents solenoid valve

// input pins
const int slideSwitch = 13;  // OFF/ON slide switch
const int incButton = 4;     // green button 
const int decButton = 5;     // yellow button
const int defaultButton = 6; // blue button 

// LCD Setup: LiquidCrystal(rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


// === button debouncing ===
const unsigned long debounceDelay = 50;
unsigned long lastDefaultPress = 0;
unsigned long lastIncPress = 0;
unsigned long lastDecPress = 0;

// === variables ===
int outputMode = 0; 				      // 0: none, 1: cycle H, 2: cycle L
bool measuring = false;				    // tracks button press duration
unsigned long pressStart = 0;		  // timestamp of when a button press started
unsigned long previousMillis = 0;	// state in cycle: 0-In, 1-Wait1, 2-Out, 3-Wait2
int state = 0;						        // True when cycle starts
bool forceCycle = false;			    // True when cycle starts
bool interrupted = false;			    // True if cycle is interrupted mid-run


// === default cycle timings (in milliseconds) ===
unsigned long h_wait1 = 45000UL, h_wait2 = 10000UL;		// cycle H: Active Contraction = 45s, Inactive Contraction = 10s
unsigned long l_wait1 = 55000UL, l_wait2 = 190000UL;	// cycle L: Active Contraction = 55s, Inactive Contraction = 190s
/* NOTE
'Active Contraction' = wait1, 'Inactive Contraction' = wait2
'UL' is used to prevent overflow: https://www.youtube.com/watch?v=qn8SP93L3iQ&ab_channel=ProgrammingElectronicsAcademy */

void setup() {
  //Serial.begin(9600);         // uncomment serial montior codes (lines 42, 72, 175) if needed
  pinMode(incButton, INPUT);
  pinMode(decButton, INPUT);
  pinMode(slideSwitch, INPUT);
  pinMode(defaultButton, INPUT);
  pinMode(pumpInPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);
 
  lcd.begin(16, 2);
  lcd.print("Select a cycle");
}


void loop() {
  int switchState = digitalRead(slideSwitch);

  // === cycle selection- default button === (counts how long the default button is pressed for cycle H and cycle L)
  if (switchState == LOW && millis() - lastDefaultPress > debounceDelay) {	  // checks if slide switch is off + button debouncing 
    if (digitalRead(defaultButton) == HIGH && !measuring) {					          // if default button is pressed, start counting duration of press
      measuring = true; 
      pressStart = millis();
    }
    if (digitalRead(defaultButton) == LOW && measuring) {					            // when default button is released, 
      measuring = false;													                            // stops measuring duration of press,
      h_wait1 = 45000UL; h_wait2 = 10000UL;									                  // (re)sets cycle H/L to their default wait times and
      l_wait1 = 55000UL; l_wait2 = 190000UL;
     
      unsigned long duration = millis() - pressStart;
      outputMode = (duration < 1500UL) ? 1 : 2;								                // if duration of press <1.5s choose cycle H, >=1.5s choose cycle L
      updateLCDtiming();
      //Serial.println(outputMode == 1 ? "H" : "L");
      lastDefaultPress = millis();
    }
  }


  // === adjusting timings- Inc/Dec Buttons ===
  if (switchState == LOW) { 														                   	          // only works if slide switch is in the OFF position
    
    // === for increase button ===
    if (digitalRead(incButton) == HIGH && millis() - lastIncPress > debounceDelay) {	// checks if inc button is pressed + button debouncing
      lastIncPress = millis();
      delay(10); 																		                                  // debounce for 10ms
      unsigned long start = millis();
      while (digitalRead(incButton) == HIGH);
      unsigned long duration = millis() - start;

      if (duration < 1500UL) {															                          // if duration of press <1.5s,
        h_wait1 += 5000UL; l_wait1 += 5000UL;											                    // increase Active Contration timing by 5s,
        updateLCDtiming();
      } else {
        h_wait2 += 5000UL; l_wait2 += 5000UL;											                    // increase Inactive Contration timing by 5s,
        updateLCDtiming();
      }
    }


    // === for decrease button ===
    if (digitalRead(decButton) == HIGH && millis() - lastDecPress > debounceDelay) {
      lastDecPress = millis();
      delay(10); 
      unsigned long start = millis();
      while (digitalRead(decButton) == HIGH);
      unsigned long duration = millis() - start;

      if (duration < 1500UL) {															                // if duration of press <1.5s,
        h_wait1 = (h_wait1 >= 5000UL) ? h_wait1 - 5000UL : 0UL;							// decrease Active Contration timing by 5s only if the time >= 5s, so it doesn’t go below zero
        l_wait1 = (l_wait1 >= 5000UL) ? l_wait1 - 5000UL : 0UL;
        updateLCDtiming();
      } else {
        h_wait2 = (h_wait2 >= 5000UL) ? h_wait2 - 5000UL : 0UL;							// decrease Inactive Contration timing by 5s 
        l_wait2 = (l_wait2 >= 5000UL) ? l_wait2 - 5000UL : 0UL;
        updateLCDtiming();
      }
    	}
  	}


  // === Start Cycle if Switch is ON ===
if (switchState == HIGH && outputMode > 0 && !forceCycle) {		      // if slide switch is ON + either cycle H/L is selected + cycle is not running,
  forceCycle = true;											                          // indicate a cycle has started,
  state = 0;                         							                  // ensure 'Pump In' is the first state, and
  previousMillis = millis();        							                  // resets timer for accurate timing
}


  // === run cycle ===
  if (outputMode == 1 && forceCycle) {							                // cycle H
    runCycle(h_wait1, h_wait2, 3000UL, 5000UL, "H", switchState);
  } else if (outputMode == 2 && forceCycle) {					              // cycle L
    runCycle(l_wait1, l_wait2, 3000UL, 5000UL, "L", switchState);
  }
}


void runCycle(unsigned long wait1, unsigned long wait2, unsigned long pulseTime1, unsigned long pulseTime2, const char* label, int switchState) {
  unsigned long currentMillis = millis();

  switch (state) {
    case 0: 													                             // 'Pump In'
      digitalWrite(pumpInPin, HIGH);						                   // pump ON
      digitalWrite(pumpOutPin, LOW);							                 // solenoid valve OFF 
      updateLCDinterruption(switchState);
      if (currentMillis - previousMillis >= pulseTime1) {		       // pump pumps water for 3s
        previousMillis = currentMillis;
        state = 1;
      }
      break;
    case 1:  													                             // 'Wait1' (active contraction)
      digitalWrite(pumpInPin, LOW);								                 // pump OFF
      digitalWrite(pumpOutPin, LOW);							                 // solenoid valve OFF
      updateLCDinterruption(switchState);
      if (currentMillis - previousMillis >= wait1) {			
        previousMillis = currentMillis;
        state = 2;
      }
      break;
    case 2: 													                            // 'Pump Out'
      digitalWrite(pumpInPin, LOW);								                // pump OFF
      digitalWrite(pumpOutPin, HIGH);							                // solenoid valve ON = open
      updateLCDinterruption(switchState);
      if (currentMillis - previousMillis >= pulseTime2) {		      // solenoid valve is open for 5s
        previousMillis = currentMillis;
        state = 3;
      }
      break;
    case 3: 													                            // 'Wait2' (inactive contraction)
      digitalWrite(pumpInPin, LOW);								                // pump OFF
      digitalWrite(pumpOutPin, LOW);							                // solenoid valve OFF

      
      if (switchState == LOW) {									                  // if the switch is in OFF position at end of wait2,
        lcd.clear();
        //Serial.println("Switch OFF during wait2, cycle interrupted.");
        lcd.print("Select a cycle");
        resetCycle();											                        // the system immediately stops, and is reset to cycle selection
        return;													
      }


      if (currentMillis - previousMillis >= wait2) {			        // finishes 'wait2'(& switch is not in OFF position)
        previousMillis = currentMillis;
        state = 0;												                        // sets the state back to "Pump In' for the cycle to continue 
       if (digitalRead(slideSwitch) == LOW) { 					          // (used 'digitalRead(slideSwitch)' instead of switchState for time-sensitive checks) 
          resetCycle();											                      // if switch is OFF at end of wait2, the system immediately stops and is reset to cycle selection
        }
      }
      break;
  }
}


// === resets cycle ===
void resetCycle() {
  forceCycle = false;
  outputMode = 0;
  state = 0;
  interrupted = false;
}


// === updates LCD ===
void updateLCDtiming(){
  // === for 'Active Contraction' ===
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Active = ");
  if (outputMode == 1) {			                                    // cycle H
    lcd.print(h_wait1 / 1000UL);	                                // converts ms to s
  } else if (outputMode == 2) {		                                // cycle L
    lcd.print(l_wait1 / 1000UL);
  }
  // === for 'Inactive Contraction' ===
  lcd.setCursor(0, 1);
  lcd.print("Inactive = ");
  if (outputMode == 1) {			                                   // cycle H
    lcd.print(h_wait2 / 1000UL);
  } else if (outputMode == 2) {		                               // cycle L
    lcd.print(l_wait2 / 1000UL);
  }
}


// === updates LCD when switch is OFF in Pump In/wait1/Pump Out ===
void updateLCDinterruption(int switchState) {
  if (switchState == LOW && !interrupted) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wait for the");
    lcd.setCursor(0, 1);
    lcd.print("next Inactive");
    interrupted = true;
  }
}



