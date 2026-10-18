#include "Display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
// Full buffer fits easily in ESP32 RAM; reuse the already installed display library.
Adafruit_SSD1306 oled(128, 64, &Wire, OLED_RESET, 400000UL, 100000UL);
bool ready = false;

void center(const char *text, int y, uint8_t size = 1) {
  oled.setTextSize(size);
  oled.setCursor((128 - int(strlen(text)) * 6 * size) / 2, y);
  oled.print(text);
  oled.setTextSize(1);
}

void percent(uint32_t n, uint32_t total) {
  if (!total) oled.print(F("--"));
  else { oled.print(uint32_t(uint64_t(n) * 100 / total)); oled.print('%'); }
}

void signedValue(float v) {
  if (v > 0.049f) oled.print('+');
  if (fabsf(v) < 0.05f) v = 0;
  oled.print(v, 1);
}

// Adapted from the teammate's three-zone bars and left-pointing indicator.
// Empty bars have no arrow: unavailable readings must not look like zero.
void bar(int x, float value, float lo, float hi, float minimum, float maximum, bool available) {
  const int top = 0, bottom = 39, bandTop = 12, bandBottom = 27, width = 20;
  oled.drawRect(x, top, width, 40, SSD1306_WHITE);
  oled.drawFastHLine(x, bandTop, width, SSD1306_WHITE);
  oled.drawFastHLine(x, bandBottom, width, SSD1306_WHITE);
  if (!available) return;
  int y;
  bool good = value >= lo && value <= hi;
  if (value < lo) {
    const int fill = constrain(int(roundf((lo - value) / (lo - minimum) * 11)), 1, 11);
    oled.fillRect(x + 1, bandBottom + 1, width - 2, fill, SSD1306_WHITE);
    y = bandBottom + fill;
  } else if (value > hi) {
    const int fill = constrain(int(roundf((value - hi) / (maximum - hi) * 11)), 1, 11);
    oled.fillRect(x + 1, bandTop - fill, width - 2, fill, SSD1306_WHITE);
    y = bandTop - fill;
  } else y = bandBottom - int((value - lo) / (hi - lo) * (bandBottom - bandTop));
  y = constrain(y, 5, bottom - 4);
  if (good) oled.fillTriangle(x + 23, y, x + 29, y - 4, x + 29, y + 4, SSD1306_WHITE);
  else oled.drawTriangle(x + 23, y, x + 29, y - 4, x + 29, y + 4, SSD1306_WHITE);
  oled.drawFastHLine(x + 29, y, 6, SSD1306_WHITE);
}

void live(const Session &s, bool target, uint32_t now) {
  const auto &d = s.detector;
  const bool available = target && !d.unsupportedMotion && !d.stale(now);
  const bool rateAvailable = available && d.bpm > 0;
  bar(2, d.bpm, RATE_LOW_BPM, RATE_HIGH_BPM, 60, 180, rateAvailable);
  bar(46, d.downMm, DEPTH_LOW_MM, DEPTH_HIGH_MM, 0, 100, available);
  oled.setCursor(0, 43); oled.print(F("BPM"));
  oled.setCursor(44, 43); oled.print(F("DEP mm"));
  oled.setCursor(0, 54);
  if (rateAvailable) oled.print(d.bpm, 0); else oled.print(F("--"));
  oled.setCursor(44, 54);
  if (available) oled.print(d.downMm, 0); else oled.print(F("--"));

  // Deliberately static concept icon, not a measured correction instruction.
  oled.setCursor(90, 0); oled.print(F("FUTURE"));
  oled.setCursor(96, 9); oled.print(F("LEAN"));
  oled.drawTriangle(108, 20, 101, 27, 115, 27, SSD1306_WHITE);
  oled.drawRect(106, 27, 5, 7, SSD1306_WHITE);
  oled.setCursor(90, 36); oled.print('#'); oled.print(d.count > 99999 ? 99999 : d.count);
  oled.setCursor(90, 46); oled.print(s.elapsed(now) / 1000UL); oled.print('s');
  oled.setCursor(90, 56);
  if (!target) oled.print(F("TARGET"));
  else if (d.unsupportedMotion) oled.print(F("RANGE"));
  else if (d.stale(now)) oled.print(F("WAIT"));
  else oled.print(F("ACTIVE"));
}

void summary(const Session &s) {
  center("SESSION COMPLETE", 0);
  oled.setCursor(0, 12); oled.print(F("N ")); oled.print(s.stats.depth.count);
  oled.print(F("  TIME ")); oled.print(s.durationMs / 1000UL); oled.print('s');
  oled.setCursor(0, 23); oled.print(F("DEPTH IN "));
  percent(s.stats.depth.inRange(), s.stats.depth.count);
  oled.setCursor(0, 34); oled.print(F("RATE IN  "));
  percent(s.stats.rate.inRange(), s.stats.rate.count);
  oled.setCursor(0, 45); oled.print(F("BOTH IN  "));
  percent(s.stats.bothInRange, s.stats.pairedCount);
  center("BTN: RESTART  1/3", 56);
}

void metricPage(const MetricStats &m, bool depth) {
  center(depth ? "DEPTH mm  50-60" : "RATE BPM  100-120", 0);
  if (!m.count) center("NO DATA", 28);
  else {
    const float reference = depth ? DEPTH_REFERENCE_MM : RATE_REFERENCE_BPM;
    oled.setCursor(0, 12); oled.print(F("AVG ")); oled.print(m.mean, 1);
    oled.print(F(" SD ")); oled.print(m.sd(), 1);
    oled.setCursor(0, 23); oled.print(F("OFFSET ")); signedValue(m.mean - reference);
    oled.print(F(" v")); oled.print(reference, 0);
    oled.setCursor(0, 34); oled.print(F("LOW ")); percent(m.low, m.count);
    oled.print(F(" HIGH ")); percent(m.high, m.count);
    oled.setCursor(0, 45); oled.print(F("IN ")); percent(m.inRange(), m.count);
    oled.print(F(" MAE ")); oled.print(m.meanAbsoluteError(), 1);
  }
  center(depth ? "BTN: RESTART  2/3" : "BTN: RESTART  3/3", 56);
}
}

bool beginDisplay() {
  if (!Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN, 100000UL)) return false;
  Wire.setTimeOut(25);
  const uint8_t addresses[] = {OLED_ADDR, OLED_ALT_ADDR};
  for (uint8_t address : addresses) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      ready = oled.begin(SSD1306_SWITCHCAPVCC, address, true, false);
      break;
    }
  }
  if (ready) oled.setTextWrap(false);
  return ready;
}

void renderDisplay(const Session &s, bool target, uint32_t now) {
  if (!ready) return;
  oled.clearDisplay(); oled.setTextColor(SSD1306_WHITE); oled.setTextSize(1);
  switch (s.screen) {
    case Screen::Start:
      center("BREATHTAKER'S CPR", 0);
      center("TRAINING DEVICE", 11);
      center("PRESS BUTTON", 29);
      center("to start", 41);
      center("Adult targets", 55);
      break;
    case Screen::Countdown: {
      center("GET INTO POSITION", 0);
      const uint32_t elapsed = now - s.phaseStartMs;
      const uint32_t left = elapsed >= COUNTDOWN_MS ? 0 : (COUNTDOWN_MS - elapsed + 999) / 1000;
      char text[8]; snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(left));
      center(text, 20, 4);
      break;
    }
    case Screen::Running: live(s, target, now); break;
    case Screen::Results:
      switch (((now - s.phaseStartMs) / RESULTS_PAGE_MS) % 3) {
        case 0: summary(s); break;
        case 1: metricPage(s.stats.depth, true); break;
        case 2: metricPage(s.stats.rate, false); break;
      }
      break;
  }
  oled.display();
}
