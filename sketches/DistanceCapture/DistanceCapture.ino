// HC-SR04 on the 5 V Pro Micro: VCC -> VCC, GND -> GND,
// TRIG -> 9, ECHO -> 10. USB serial -> Mac CSV capture task.
// Raw readings only: no smoothing, stroke detection, or depth calibration.
#define TRIG_PIN 9
#define ECHO_PIN 10
#define BUZZER_PIN 4
#define ECHO_TIMEOUT_US 30000UL
#define SAMPLE_INTERVAL_MS 70UL

unsigned long lastSampleMs = 0;
bool serialWasOpen = false;

void setup() {
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
  if (!Serial) {
    serialWasOpen = false;
    return;
  }
  if (!serialWasOpen) {
    Serial.println(F("timestamp_ms,echo_duration_us,distance_mm,valid"));
    serialWasOpen = true;
  }

  const unsigned long now = millis();
  if (now - lastSampleMs < SAMPLE_INTERVAL_MS) return;
  // Schedule from this measurement, so delays cannot cause catch-up bursts.
  lastSampleMs = now;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  const unsigned long timestampMs = millis();
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  // Same nominal conversion as DistanceTest: echo / 58 gives centimeters.
  const float distanceMm = echoUs / 5.8f;
  const bool valid = echoUs != 0 && distanceMm >= 20.0f && distanceMm <= 4000.0f;
  Serial.print(timestampMs); // Board uptime at trigger, not Mac arrival time.
  Serial.print(',');
  Serial.print(echoUs);
  Serial.print(',');
  if (echoUs != 0) Serial.print(distanceMm, 1);
  Serial.print(',');
  Serial.println(valid ? 1 : 0);
}
