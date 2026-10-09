#include <Arduino.h>

// HC-SR04 wiring for an Uno R3:
// Vcc -> 5V, Gnd -> GND, Trig -> D9, Echo -> D10.
// Open the serial monitor at 115200 baud.
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const unsigned long ECHO_TIMEOUT_US = 30000;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  Serial.println(F("HC-SR04 test | Trig=D9 | Echo=D10 | distance in cm"));
}

void loop() {
  // A 10-microsecond HIGH pulse requests one measurement.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the returning HIGH pulse. A timeout returns 0, not a distance.
  const unsigned long echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);

  if (echoUs == 0) {
    Serial.println(F("NO ECHO | check wiring, target position, and range"));
  } else {
    // Sound travels about 0.0343 cm/us at room temperature.
    // Divide by 2 because the sound travels to the target AND back.
    const float distanceCm = echoUs * 0.0343f / 2.0f;
    Serial.print(F("Distance: "));
    Serial.print(distanceCm, 1);
    Serial.print(F(" cm | echo: "));
    Serial.print(echoUs);
    Serial.print(F(" us"));
    if (distanceCm < 2.0f || distanceCm > 400.0f) {
      Serial.print(F(" | OUT OF RATED RANGE (2-400 cm)"));
    }
    Serial.println();
  }

  // Keep this starter test simple and leave time between sound bursts.
  delay(250);
}
