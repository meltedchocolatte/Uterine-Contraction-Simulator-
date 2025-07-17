
/*For testing push buttons, slide switch and LCD*/

#include <LiquidCrystal.h>

// Pin setup
const int incButton = 4;     // Button A
const int decButton = 5;     // Button B
const int defaultButton = 6;
const int slideSwitch = 13;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  pinMode(incButton, INPUT);
  pinMode(decButton, INPUT);
  pinMode(defaultButton, INPUT);
  pinMode(slideSwitch, INPUT);

  lcd.begin(16, 2);
  lcd.print("Testing buttons");
  delay(1000);
  lcd.clear();
}

void loop() {
  lcd.setCursor(0, 0);

  if (digitalRead(incButton) == HIGH) {
    lcd.print("Button A pressed ");
  } else if (digitalRead(decButton) == HIGH) {
    lcd.print("Button B pressed ");
  } else if (digitalRead(defaultButton) == HIGH) {
    lcd.print("Default pressed   ");
  } else if (digitalRead(slideSwitch) == HIGH) {
    lcd.print("Slide: HIGH       ");
  } else {
    lcd.print("No button pressed ");
  }

  delay(200);  // Small delay to avoid LCD flicker
}
