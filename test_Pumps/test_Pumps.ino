/*To test the functionality of the pump and the solenoid after being connected to the MOSFED circuit*/

const int pumpInPin = 2;
const int pumpOutPin = 3;

void setup() {
  pinMode(pumpInPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("Pump Test Starting...");
}

void loop() {
  // Pump In
  Serial.println("Pump In ON");
  digitalWrite(pumpInPin, HIGH);
  delay(2000);

  Serial.println("Pump In OFF");
  digitalWrite(pumpInPin, LOW);
  delay(1000);

  // Pump Out
  Serial.println("Pump Out ON");
  digitalWrite(pumpOutPin, HIGH);
  delay(2000);

  Serial.println("Pump Out OFF");
  digitalWrite(pumpOutPin, LOW);
  delay(1000);
}
