/*For testing slide switch*/

const int switchPin = 13;   // Pin where the switch is connected
int switchState = 0;       // Variable for reading the switch status

void setup() {
  pinMode(switchPin, INPUT);    // Set pin as input
  Serial.begin(9600);           // Start serial monitor
}

void loop() {
  switchState = digitalRead(switchPin);  // Read the state of the switch

  if (switchState == HIGH) {
    Serial.println("Switch is ON (pressed)");
  } else {
    Serial.println("Switch is OFF (not pressed)");
  }

  delay(500);  // Wait for half a second
}
