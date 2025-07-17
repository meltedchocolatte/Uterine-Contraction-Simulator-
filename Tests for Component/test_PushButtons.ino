/*For testing push buttons*/

// Pin setup
const int incButton = 4;     // Button A
const int decButton = 5;     // Button B
const int defaultButton = 6;

void setup() {
  pinMode(incButton, INPUT);      
  pinMode(decButton, INPUT);
  pinMode(defaultButton, INPUT);

  Serial.begin(9600);
}

void loop() {
  if (digitalRead(incButton) == HIGH) {   
    Serial.println("Inc button pressed");
    delay(200); // debounce delay
  }

  if (digitalRead(decButton) == HIGH) {
    Serial.println("Dec button pressed");
    delay(200); // debounce delay
  }

  if (digitalRead(defaultButton) == HIGH) {
    Serial.println("Default button pressed");
    delay(200); // debounce delay
  }
}
