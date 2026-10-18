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

// Classic font at size 2: each character occupies 12 x 16 pixels.
void valueAt(int x, int y, const char *text, uint8_t size = 2) {
  oled.setTextSize(size);
  oled.setCursor(x, y);
  oled.print(text);
  oled.setTextSize(1);
}

void formatPercent(char *text, size_t capacity, uint32_t n, uint32_t total) {
  if (!total) snprintf(text, capacity, "--");
  else snprintf(text, capacity, "%lu%%", static_cast<unsigned long>(uint64_t(n) * 100 / total));
}

// Thin bars: filled high zone in the physical yellow band, outlined target
// zone, filled low zone in blue. OLED colors are fixed by the panel, not code.
void bar(int x, float value, float lo, float hi, float minimum, float maximum, bool available) {
  const int width = 8, height = 39;
  const int bandTop = OLED_YELLOW_ROWS, bandBottom = 28;
  static_assert(OLED_YELLOW_ROWS >= 8 && OLED_YELLOW_ROWS <= 20,
                "Yellow band must leave room for the target zone.");
  oled.fillRect(x, 0, width, bandTop, SSD1306_WHITE);
  oled.drawRect(x, bandTop, width, bandBottom - bandTop, SSD1306_WHITE);
  oled.fillRect(x, bandBottom, width, height - bandBottom, SSD1306_WHITE);
  if (!available) return;
  int y;
  if (value > hi) {
    const float fraction = constrain((value - hi) / (maximum - hi), 0.0f, 1.0f);
    y = bandTop - 5 - int(roundf(fraction * (bandTop - 9)));
  } else if (value < lo) {
    const float fraction = constrain((lo - value) / (lo - minimum), 0.0f, 1.0f);
    y = bandBottom + 3 + int(roundf(fraction * (height - bandBottom - 7)));
  } else {
    y = bandBottom - 2 - int(roundf((value - lo) / (hi - lo) * (bandBottom - bandTop - 3)));
  }
  // Long outlined arrow: 23 pixels from tip to tail, with a 3-pixel shaft.
  const int tip = x + width + 2, neck = tip + 8, tail = tip + 22;
  oled.drawLine(tip, y, neck, y - 3, SSD1306_WHITE);
  oled.drawLine(tip, y, neck, y + 3, SSD1306_WHITE);
  oled.drawFastVLine(neck, y - 3, 3, SSD1306_WHITE);
  oled.drawFastVLine(neck, y + 1, 3, SSD1306_WHITE);
  oled.drawFastHLine(neck, y - 1, tail - neck + 1, SSD1306_WHITE);
  oled.drawFastHLine(neck, y + 1, tail - neck + 1, SSD1306_WHITE);
  oled.drawFastVLine(tail, y - 1, 3, SSD1306_WHITE);
}

void live(const Session &s, bool target, uint32_t now) {
  const auto &d = s.detector;
  const bool available = target && !d.unsupportedMotion && !d.stale(now);
  const bool rateAvailable = available && d.bpm > 0;
  bar(2, d.downMm, DEPTH_LOW_MM, DEPTH_HIGH_MM, 0, 120, available);
  bar(46, d.bpm, RATE_LOW_BPM, RATE_HIGH_BPM, 30, 215, rateAvailable);
  oled.setCursor(0, 40); oled.print(F("DEP mm"));
  oled.setCursor(44, 40); oled.print(F("BPM"));
  char text[16];
  if (available) snprintf(text, sizeof(text), "%.0f", d.downMm);
  else snprintf(text, sizeof(text), "--");
  valueAt(0, 48, text);
  if (rateAvailable) snprintf(text, sizeof(text), "%.0f", d.bpm);
  else snprintf(text, sizeof(text), "--");
  valueAt(44, 48, text);

  // Static future-feature icon, explicitly not a measured correction.
  oled.setCursor(92, 0); oled.print(F("FUTURE"));
  oled.setCursor(98, 9); oled.print(F("LEAN"));
  oled.drawFastHLine(103, 26, 13, SSD1306_WHITE);
  oled.drawFastVLine(109, 20, 13, SSD1306_WHITE);
  oled.fillTriangle(109, 20, 107, 22, 111, 22, SSD1306_WHITE);
  oled.fillTriangle(109, 32, 107, 30, 111, 30, SSD1306_WHITE);
  oled.fillTriangle(103, 26, 105, 24, 105, 28, SSD1306_WHITE);
  oled.fillTriangle(115, 26, 113, 24, 113, 28, SSD1306_WHITE);
  oled.setCursor(92, 40); oled.print(F("TIME s"));
  const uint32_t seconds = s.elapsed(now) / 1000UL;
  snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(seconds));
  valueAt(92, 48, text, seconds <= 999 ? 2 : 1);
}

// Three large rows occupy the blue band; titles occupy the yellow band.
void statRow(const char *label, const char *value, int y) {
  valueAt(0, y, label);
  valueAt(128 - int(strlen(value)) * 12, y, value);
}

void summary(const Session &s) {
  center("GENERAL STATS  1/3", 3);
  char text[12];
  formatPercent(text, sizeof(text), s.stats.rate.inRange(), s.stats.rate.count);
  statRow("BPM", text, 16);
  formatPercent(text, sizeof(text), s.stats.depth.inRange(), s.stats.depth.count);
  statRow("DEP", text, 32);
  formatPercent(text, sizeof(text), s.stats.bothInRange, s.stats.pairedCount);
  statRow("BOTH", text, 48);
}

void metricPage(const MetricStats &m, bool depth) {
  center(depth ? "DEPTH mm 50-60  3/3" : "BPM 100-120  2/3", 3);
  char text[12];
  if (m.count) snprintf(text, sizeof(text), "%.1f", m.mean);
  else snprintf(text, sizeof(text), "--");
  statRow("AVG", text, 16);
  formatPercent(text, sizeof(text), m.low, m.count);
  statRow("LOW", text, 32);
  formatPercent(text, sizeof(text), m.high, m.count);
  statRow("HIGH", text, 48);
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
        case 1: metricPage(s.stats.rate, false); break;
        case 2: metricPage(s.stats.depth, true); break;
      }
      break;
  }
  oled.display();
}
