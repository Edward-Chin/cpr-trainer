#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// HC-SR04 wiring for an Uno R3:
// Vcc -> 5V, Gnd -> GND, Trig -> D9, Echo -> D10.
// SSD1306 OLED: VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5.
// Libraries: Adafruit SSD1306 and its dependencies (GFX and BusIO).
// Open the serial monitor at 115200 baud.
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const unsigned long ECHO_TIMEOUT_US = 30000;

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
  display.print(F("HC-SR04 DISTANCE"));
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
  Serial.println(F("HC-SR04 test | Trig=D9 | Echo=D10 | distance in cm"));
  setupDisplay();
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
  // Sound travels about 0.0343 cm/us at room temperature.
  // Divide by 2 because it travels to the target AND back.
  // A zero echo is handled as a timeout below, not as a valid distance.
  const float distanceCm = echoUs * 0.0343f / 2.0f;

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

  // Keep this starter test simple and leave time between sound bursts.
  delay(250);
}
