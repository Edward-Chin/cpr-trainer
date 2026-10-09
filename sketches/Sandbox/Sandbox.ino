#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// HC-SR04 wiring for an Uno R3:
// Vcc -> 5V, Gnd -> GND, Trig -> D9, Echo -> D10.
// SSD1306 OLED: VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5.
// LEDs: D4 green, D5 yellow, D6 red; each pin -> 330 ohm -> anode.
// All LED cathodes -> GND. Smaller distance simulates rising water.
// Libraries: Adafruit SSD1306 and its dependencies (GFX and BusIO).
// Open the serial monitor at 115200 baud.
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int GREEN_PIN = 4;
const int YELLOW_PIN = 5;
const int RED_PIN = 6;
const unsigned long ECHO_TIMEOUT_US = 30000;
const unsigned long SAMPLE_INTERVAL_MS = 250;
const unsigned long FAULT_BLINK_MS = 250;
const float ALARM_ENTER_CM = 10.0f;
const float ALARM_CLEAR_CM = 12.0f;
const float WARNING_ENTER_CM = 20.0f;
const float WARNING_CLEAR_CM = 22.0f;

enum LevelState { NORMAL, WARNING, ALARM };
LevelState levelState = NORMAL; // Last valid state is retained during a fault.
bool sensorFault = true; // No valid reading yet.
unsigned long lastSampleMs = 0;

void updateState(unsigned long echoUs, float distanceCm) {
  sensorFault = echoUs == 0 || distanceCm < 2.0f || distanceCm > 400.0f;
  if (sensorFault) {
    return;
  }

  // Separate entry/exit thresholds prevent chatter near a boundary.
  // Check alarm first so a sudden jump from far to near goes straight to red.
  if (distanceCm < ALARM_ENTER_CM ||
      (levelState == ALARM && distanceCm <= ALARM_CLEAR_CM)) {
    levelState = ALARM;
  } else if (distanceCm <= WARNING_ENTER_CM ||
             (levelState != NORMAL && distanceCm <= WARNING_CLEAR_CM)) {
    levelState = WARNING;
  } else {
    levelState = NORMAL;
  }
}

const __FlashStringHelper* stateLabel() {
  if (sensorFault) return F("SENSOR FAULT");
  if (levelState == ALARM) return F("ALARM");
  if (levelState == WARNING) return F("WARNING");
  return F("NORMAL");
}

void updateLeds() {
  if (sensorFault) {
    const bool on = (millis() / FAULT_BLINK_MS) % 2 == 0;
    digitalWrite(GREEN_PIN, on);
    digitalWrite(YELLOW_PIN, on);
    digitalWrite(RED_PIN, on);
    return;
  }
  digitalWrite(GREEN_PIN, levelState == NORMAL);
  digitalWrite(YELLOW_PIN, levelState == WARNING);
  digitalWrite(RED_PIN, levelState == ALARM);
}

// This four-pin display has no separate reset pin (-1).
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool displayReady = false;

void setupDisplay() {
  Wire.begin();
  // Avoid hanging forever if the I2C bus gets stuck.
  Wire.setWireTimeout(25000, true);

  // The board's address jumper selects either 0x3C or 0x3D.
  uint8_t address = 0;
  for (uint8_t candidate = 0x3C; candidate <= 0x3D; ++candidate) {
    Wire.beginTransmission(candidate);
    if (Wire.endTransmission() == 0) {
      address = candidate;
      break;
    }
  }
  if (address == 0) {
    Serial.println(F("OLED not detected at 0x3C/0x3D; check wiring, then reset."));
    return; // Keep the sensor and serial output working without a display.
  }

  // SSD1306 uses an internal charge pump to power the OLED panel.
  if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
    Serial.println(F("OLED buffer allocation failed; continuing with serial."));
    return;
  }
  displayReady = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  Serial.print(F("OLED initialized at 0x"));
  Serial.println(address, HEX);
}

void showReading(unsigned long echoUs, float distanceCm) {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(stateLabel());
  display.setCursor(0, 22);

  if (echoUs == 0) {
    display.print(F("NO ECHO"));
    display.setCursor(0, 36);
    display.print(F("Check target / wires"));
  } else if (distanceCm < 2.0f || distanceCm > 400.0f) {
    display.print(F("OUT OF RANGE"));
    display.setCursor(0, 36);
    display.print(F("Valid: 2-400 cm"));
  } else {
    display.setTextSize(2);
    display.print(distanceCm, 1);
    display.print(F(" cm"));
  }

  display.setTextSize(1);
  display.setCursor(0, 52);
  display.print(F("Echo: "));
  display.print(echoUs);
  display.print(F(" us"));
  display.display(); // Send the finished image to the screen.
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  updateLeds();
  Serial.println(F("Level demo | Trig=D9 Echo=D10 | Green=D4 Yellow=D5 Red=D6"));
  Serial.println(F("Alarm <10 cm, clears >12; warning <=20 cm, clears >22."));
  setupDisplay();
  lastSampleMs = millis() - SAMPLE_INTERVAL_MS; // Take the first reading immediately.
}

void loop() {
  updateLeds();
  const unsigned long now = millis();
  // Unsigned subtraction also works when millis() wraps around.
  if (now - lastSampleMs < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleMs = now;

  // A 10-microsecond HIGH pulse requests one measurement.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the returning HIGH pulse. A timeout returns 0, not a distance.
  const unsigned long echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  // Sound travels about 0.0343 cm/us at room temperature.
  // Divide by 2 because it travels to the target AND back.
  // A zero echo is handled as a timeout below, not as a valid distance.
  const float distanceCm = echoUs * 0.0343f / 2.0f;
  updateState(echoUs, distanceCm);
  updateLeds();

  Serial.print(stateLabel());
  Serial.print(F(" | "));

  if (echoUs == 0) {
    Serial.println(F("NO ECHO | check wiring, target position, and range"));
  } else {
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
  showReading(echoUs, distanceCm);

  // No delay: fault blinking continues between measurements.
}
