// OLED UI demo for the 5 V / 16 MHz ATmega32U4 Pro Micro.
// Assumes a 128x64 SSD1306 I2C OLED; confirm module voltage compatibility.
// OLED SDA -> 2, SCL -> 3, GND -> GND; power per the module specification.
// Button -> pin 5 and GND (INPUT_PULLUP). Buzzer driver on pin 4 stays off.
// Countdowns and feedback are demonstrations: no sensors are read/calibrated.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
#define OLED_ALT_ADDR 0x3D
// Hardware I2C pins are fixed by the Pro Micro board core (Wire).
#define OLED_SDA_PIN 2
#define OLED_SCL_PIN 3
#define BUZZER_PIN 4

#define BUTTON_PIN 5
#define POSITION_SECONDS 10     // "GET INTO POSITION" countdown
#define CALIBRATE_SECONDS 5     // "CALIBRATING" countdown

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum SpeedState { SPEED_GOOD, SPEED_TOO_FAST, SPEED_TOO_SLOW };
enum DepthState { DEPTH_GOOD, DEPTH_TOO_DEEP, DEPTH_TOO_SHALLOW };
enum AngleState { ANGLE_UP, ANGLE_DOWN, ANGLE_LEFT, ANGLE_RIGHT, ANGLE_CENTER };
enum Mode { MODE_IDLE, MODE_POSITION, MODE_CALIBRATING, MODE_RUN };

// Explicit declarations keep Arduino's generated prototypes after these enums.
void drawSpeed(SpeedState s);
void drawDepth(DepthState d);
void drawArrow(AngleState a);
void updateDisplay(SpeedState s, DepthState d, AngleState a);
void startPhase(Mode m);

Mode mode = MODE_IDLE;
unsigned long phaseStart = 0;
int lastShown = -1;

// ---------- Button (debounced, true once per press) ----------
bool buttonPressed() {
  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long lastChange = 0;

  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastReading) {
    lastChange = millis();
    lastReading = reading;
  }
  if (millis() - lastChange > 40 && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) return true;  // pressed (pulled to GND)
  }
  return false;
}

// ---------- Status screen drawing ----------
void drawStatus(int y, const __FlashStringHelper* label,
                const __FlashStringHelper* value, bool good) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, y);
  display.print(label);

  if (!good) {
    display.fillRect(0, y + 10, 84, 11, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  }
  display.setCursor(3, y + 12);
  display.print(value);
  display.setTextColor(SSD1306_WHITE);
}

void drawSpeed(SpeedState s) {
  switch (s) {
    case SPEED_GOOD:     drawStatus(0, F("SPEED"), F("GOOD"), true); break;
    case SPEED_TOO_FAST: drawStatus(0, F("SPEED"), F("TOO FAST"), false); break;
    case SPEED_TOO_SLOW: drawStatus(0, F("SPEED"), F("TOO SLOW"), false); break;
  }
}

void drawDepth(DepthState d) {
  switch (d) {
    case DEPTH_GOOD:        drawStatus(34, F("DEPTH"), F("GOOD"), true); break;
    case DEPTH_TOO_DEEP:    drawStatus(34, F("DEPTH"), F("TOO DEEP"), false); break;
    case DEPTH_TOO_SHALLOW: drawStatus(34, F("DEPTH"), F("TOO SHALLOW"), false); break;
  }
}

void drawArrow(AngleState a) {
  const int cx = 108, cy = 38;

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(93, 0);
  display.print(F("ANGLE"));

  switch (a) {
    case ANGLE_UP:
      display.fillTriangle(cx, cy - 18, cx - 13, cy, cx + 13, cy, SSD1306_WHITE);
      display.fillRect(cx - 4, cy, 9, 18, SSD1306_WHITE);
      break;
    case ANGLE_DOWN:
      display.fillTriangle(cx, cy + 18, cx - 13, cy, cx + 13, cy, SSD1306_WHITE);
      display.fillRect(cx - 4, cy - 18, 9, 18, SSD1306_WHITE);
      break;
    case ANGLE_LEFT:
      display.fillTriangle(cx - 18, cy, cx, cy - 13, cx, cy + 13, SSD1306_WHITE);
      display.fillRect(cx, cy - 4, 18, 9, SSD1306_WHITE);
      break;
    case ANGLE_RIGHT:
      display.fillTriangle(cx + 18, cy, cx, cy - 13, cx, cy + 13, SSD1306_WHITE);
      display.fillRect(cx - 18, cy - 4, 18, 9, SSD1306_WHITE);
      break;
    case ANGLE_CENTER:
      display.drawCircle(cx, cy, 16, SSD1306_WHITE);
      display.fillCircle(cx, cy, 7, SSD1306_WHITE);
      break;
  }
}

void updateDisplay(SpeedState s, DepthState d, AngleState a) {
  display.clearDisplay();
  drawSpeed(s);
  drawDepth(d);
  drawArrow(a);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(F("DEMO"));
  display.drawLine(88, 0, 88, 63, SSD1306_WHITE);
  display.display();
}

// ---------- Idle and countdown screens ----------
void drawIdle() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 10);
  display.print(F("PRESS"));
  display.setCursor(16, 30);
  display.print(F("BUTTON"));
  display.setTextSize(1);
  display.setCursor(13, 54);
  display.print(F("OLED DEMO - start"));
  display.display();
}

// Shared countdown screen: small label at top (centered at labelX), big number below
void drawCountdown(const __FlashStringHelper* label, int labelX, int secondsLeft) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(labelX, 4);
  display.print(label);

  display.setTextSize(5);
  int digits = (secondsLeft >= 10) ? 2 : 1;
  int x = (SCREEN_WIDTH - digits * 30) / 2;   // each size-5 character is 30px wide
  display.setCursor(x, 20);
  display.print(secondsLeft);
  display.display();
}

void startPhase(Mode m) {
  mode = m;
  phaseStart = millis();
  lastShown = -1;   // forces the first draw of the new phase
}

// Runs one countdown phase; returns true when it has finished
bool runCountdown(const __FlashStringHelper* label, int labelX, int totalSeconds) {
  unsigned long elapsed = millis() - phaseStart;
  if (elapsed >= (unsigned long)totalSeconds * 1000UL) return true;

  int left = totalSeconds - (int)(elapsed / 1000UL);
  if (left != lastShown) {      // only redraw when the number changes
    lastShown = left;
    drawCountdown(label, labelX, left);
  }
  return false;
}

bool oledAcknowledges(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void setup() {
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  const unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 3000UL) {}

  Wire.begin();
  Wire.setClock(100000UL);
  Wire.setWireTimeout(25000UL, true);
  uint8_t address = OLED_ADDR;
  if (!oledAcknowledges(address)) {
    address = OLED_ALT_ADDR;
    if (!oledAcknowledges(address)) {
      for (;;) {
        Serial.println(F("OLED not found at 0x3C/0x3D: check power, SDA=2, SCL=3."));
        delay(2000);
      }
    }
  }

  // ACK confirms a responding device, not its controller type or resolution.
  if (!display.begin(SSD1306_SWITCHCAPVCC, address, true, false)) {
    for (;;) {
      Serial.println(F("SSD1306 framebuffer allocation failed."));
      delay(2000);
    }
  }
  display.setTextWrap(false);
  Serial.print(F("OLED demo at 0x"));
  Serial.println(address, HEX);
  Serial.println(F("Button: pin 5 to GND. Feedback/calibration are simulated."));
  drawIdle();
}

void loop() {
  bool pressed = buttonPressed();

  switch (mode) {
    case MODE_IDLE:
      if (pressed) startPhase(MODE_POSITION);
      break;

    case MODE_POSITION:
      // "GET INTO POSITION" = 17 chars x 6px = 102px wide, so x = 13 centers it
      if (runCountdown(F("GET INTO POSITION"), 13, POSITION_SECONDS)) {
        startPhase(MODE_CALIBRATING);
      }
      break;

    case MODE_CALIBRATING:
      // "CALIBRATING" = 11 chars x 6px = 66px wide, so x = 31 centers it
      if (runCountdown(F("CALIBRATING"), 31, CALIBRATE_SECONDS)) {
        mode = MODE_RUN;
        lastShown = -1;
      }
      break;

    case MODE_RUN: {
      // Pressing the button again restarts the whole sequence
      if (pressed) {
        startPhase(MODE_POSITION);
        break;
      }

      // ---- DEMO: cycles through all states every 2 s. Replace with your logic. ----
      static unsigned long lastUpdate = 0;
      static byte i = 0;
      static bool first = true;
      if (lastShown == -1) { first = true; lastShown = 0; }  // redraw right after calibration
      if (first || millis() - lastUpdate >= 2000) {
        first = false;
        lastUpdate = millis();
        SpeedState speedState = (SpeedState)(i % 3);
        DepthState depthState = (DepthState)((i + 1) % 3);
        AngleState angleState = (AngleState)(i % 5);
        i++;
        updateDisplay(speedState, depthState, angleState);
      }
      // -----------------------------------------------------------------------------
      break;
    }
  }
}
