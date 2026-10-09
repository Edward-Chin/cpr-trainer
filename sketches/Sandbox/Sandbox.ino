#include <Arduino.h>

// Workflow check: serial heartbeat; also blink if this board defines an LED.
// No sensor wiring required. LED polarity/location varies by board.
const unsigned long intervalMs = 1000;
unsigned long lastTick = 0;
unsigned long tickCount = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  // Do not wait indefinitely for a serial monitor to connect.
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif
}

void loop() {
  const unsigned long now = millis();
  if (now - lastTick >= intervalMs) {
    lastTick = now;
    ++tickCount;
    ledOn = !ledOn;
#ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);
#endif
    Serial.print("Sandbox alive | tick=");
    Serial.print(tickCount);
    Serial.print(" | uptime_ms=");
    Serial.println(now);
  }
}
