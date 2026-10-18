#include "Hardware.h"
#include <esp_timer.h>

namespace {
esp_timer_handle_t buzzerTimer = nullptr;
portMUX_TYPE buzzerLock = portMUX_INITIALIZER_UNLOCKED;
bool buzzerEnabled = false, buzzerHigh = false;
int64_t beatEpochUs = 0;

// ESP timer task is independent of OLED I2C transfers and pulseIn().
// 5 ms service resolution; no catch-up burst after a delayed callback.
void buzzerTick(void *) {
  const int64_t now = esp_timer_get_time();
  portENTER_CRITICAL(&buzzerLock);
  const bool high = buzzerEnabled &&
      uint64_t(now - beatEpochUs) % (60000000ULL / METRONOME_BPM) < CHIRP_US;
  if (high != buzzerHigh) {
    buzzerHigh = high;
    digitalWrite(BUZZER_PIN, high ? HIGH : LOW);
  }
  portEXIT_CRITICAL(&buzzerLock);
}
}

void beginHardware() {
  digitalWrite(BUZZER_PIN, LOW); pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW); pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  esp_timer_create_args_t args = {};
  args.callback = buzzerTick;
  args.name = "metronome";
  if (esp_timer_create(&args, &buzzerTimer) != ESP_OK) {
    buzzerTimer = nullptr;
    Serial.println(F("# Metronome timer creation failed."));
  } else if (esp_timer_start_periodic(buzzerTimer, 5000) != ESP_OK) {
    esp_timer_delete(buzzerTimer);
    buzzerTimer = nullptr;
    Serial.println(F("# Metronome timer start failed."));
  }
}

void setMetronome(bool enabled) {
  portENTER_CRITICAL(&buzzerLock);
  if (enabled && !buzzerEnabled) beatEpochUs = esp_timer_get_time();
  buzzerEnabled = enabled && buzzerTimer != nullptr;
  if (!buzzerEnabled) {
    buzzerHigh = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
  portEXIT_CRITICAL(&buzzerLock);
}

bool buttonPressed() {
  static bool last = HIGH, stable = HIGH;
  static uint32_t changed = 0;
  const bool reading = digitalRead(BUTTON_PIN);
  const uint32_t now = millis();
  if (reading != last) { last = reading; changed = now; }
  if (uint32_t(now - changed) >= BUTTON_DEBOUNCE_MS && reading != stable) {
    stable = reading;
    return stable == LOW;
  }
  return false;
}

bool readDistance(DistanceSample &sample) {
  static uint32_t previousMs = 0;
  const uint32_t now = millis();
  if (uint32_t(now - previousMs) < SAMPLE_INTERVAL_MS) return false;
  previousMs = now;
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  sample.timestampMs = millis();
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10); digitalWrite(TRIG_PIN, LOW);
  sample.echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  sample.gapMm = sample.echoUs / 5.8f;
  sample.valid = sample.echoUs != 0 && sample.gapMm >= MIN_GAP_MM && sample.gapMm <= MAX_GAP_MM;
  return true;
}

void logSample(const DistanceSample &sample, const StrokeResult &event, const Session &session) {
  if (!Serial) return;
  const auto &d = session.detector;
  const bool live = session.screen == Screen::Running && sample.valid &&
                    !d.unsupportedMotion && !d.stale(sample.timestampMs);
  Serial.print(sample.timestampMs); Serial.print(',');
  if (sample.echoUs) Serial.print(sample.gapMm, 1);
  Serial.print(','); Serial.print(sample.valid ? 1 : 0); Serial.print(',');
  if (event.down) Serial.print(F("down")); else if (event.up) Serial.print(F("up"));
  Serial.print(','); if (live) Serial.print(d.downMm, 1);
  Serial.print(','); if (live && d.haveUp) Serial.print(d.upMm, 1);
  Serial.print(','); if (live && d.bpm > 0) Serial.print(d.bpm, 1);
  Serial.print(','); if (live && d.bpm > 0) Serial.print(d.bpm / 60.0f, 3);
  Serial.print(','); Serial.println(d.count);
}
