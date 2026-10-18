// TMB12A05 active buzzer driven by a 2N3904 NPN transistor:
// VCC (5 V) -> buzzer +; buzzer - -> collector; emitter -> GND.
// Pin 4 -> 1 kohm -> base; base -> 10 kohm -> GND.
// Flyback diode across buzzer: cathode/stripe to +, anode to -.
constexpr uint8_t BUZZER_PIN = 4;
constexpr unsigned long BEEP_MS = 150;
constexpr unsigned long QUIET_MS = 1850;

void setup() {
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);
  // Starts without a serial monitor after a short bounded wait.
  const unsigned long started = millis();
  while (!Serial && millis() - started < 3000UL) {}
  Serial.println(F("Buzzer test: pin 4, 150 ms beep every ~2 seconds."));
}

void loop() {
  // No tone(): this active buzzer generates its own audio frequency.
  digitalWrite(BUZZER_PIN, HIGH);
  delay(BEEP_MS);
  digitalWrite(BUZZER_PIN, LOW);
  // Print only after turning off so serial output cannot prolong the beep.
  Serial.println(F("Beep pulse completed; pin 4 is LOW."));
  delay(QUIET_MS);
}
