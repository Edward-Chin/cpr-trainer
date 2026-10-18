// Integrated ESP32 prototype. Detector is unchanged from StrokeTest.
// Hardware: Config.h / Hardware.cpp; state and stats: Session.h; UI: Display.cpp.
#include "Hardware.h"
#include "Display.h"

Session session;
DistanceSample latestSample;
uint32_t lastDisplayMs = 0;

void setup() {
  beginHardware();
  if (!beginDisplay()) Serial.println(F("# OLED unavailable; check SDA21/SCL22, power and address."));
  Serial.println(F("# CPRTrainer: button starts countdown, ends session, then restarts."));
  Serial.println(F("timestamp_ms,gap_mm,in_range,event,down_mm,up_mm,bpm,hz,count"));
  renderDisplay(session, false, millis());
}

void loop() {
  const Screen previous = session.screen;
  const uint32_t now = millis();
  if (buttonPressed()) session.press(now);
  session.tick(now);
  if (session.screen != previous) {
    setMetronome(session.screen == Screen::Running);
    Serial.print(F("# State: "));
    Serial.println(session.screen == Screen::Countdown ? "countdown" :
                   session.screen == Screen::Running ? "running" : "results");
  }

  // Keep raw capture available in every state; only Running feeds the detector.
  const bool sampled = readDistance(latestSample);
  if (sampled) {
    const StrokeResult event = session.sample(latestSample.timestampMs, latestSample.gapMm, latestSample.valid);
    logSample(latestSample, event, session);
  }
  // Refresh just after a measurement to leave time before the next trigger.
  if (session.screen != previous ||
      (sampled && uint32_t(millis() - lastDisplayMs) >= DISPLAY_INTERVAL_MS)) {
    renderDisplay(session, latestSample.valid, millis());
    lastDisplayMs = millis();
  }
  delay(1);
}
