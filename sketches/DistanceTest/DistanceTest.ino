// HC-SR04 on a confirmed 5 V Pro Micro:
// VCC -> VCC, GND -> GND, TRIG -> 9, ECHO -> 10.
constexpr uint8_t TRIG_PIN = 9;
constexpr uint8_t ECHO_PIN = 10;
constexpr unsigned long ECHO_TIMEOUT_US = 30000UL;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
  // Native USB serial may not be open yet; don't wait forever without a PC.
  const unsigned long started = millis();
  while (!Serial && millis() - started < 3000UL) {}
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  Serial.print(millis());
  Serial.print(F(" ms | "));
  if (echoUs == 0) {
    Serial.println(F("NO ECHO / TIMEOUT"));
  } else {
    const float distanceCm = echoUs / 58.0f;
    Serial.print(distanceCm, 1);
    Serial.print(F(" cm | echo "));
    Serial.print(echoUs);
    Serial.print(F(" us"));
    if (distanceCm < 2.0f || distanceCm > 400.0f) {
      Serial.print(F(" | OUT OF SPEC RANGE"));
    }
    Serial.println();
  }
  // More than the datasheet's 60 ms minimum between trigger pulses.
  // Blocking measurement is deliberate for this standalone test only.
  delay(70);
}
