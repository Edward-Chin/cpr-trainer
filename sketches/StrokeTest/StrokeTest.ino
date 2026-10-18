// Live motion measurements, not validated CPR depth or proficiency scoring.
// Classic ESP32: HC-SR04 TRIG 5, ECHO 18 through a 5 V -> 3 V divider.
// SSD1306 OLED: SDA 21, SCL 22 (3.3 V logic). Button: 4 to GND.
// Transistor buzzer driver: GPIO 2 (held off); do not power the buzzer from GPIO.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "StrokeDetector.h"

#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 2
#define BUTTON_PIN 4
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define I2C_CLOCK_HZ 100000UL
#define I2C_TIMEOUT_MS 25
#define OLED_RESET -1
#define OLED_ADDR 0x3C
#define OLED_ALT_ADDR 0x3D
#define SAMPLE_INTERVAL_MS 70UL
#define ECHO_TIMEOUT_US 30000UL
#define DISPLAY_INTERVAL_MS 210UL

Adafruit_SSD1306 display(128,64,&Wire,OLED_RESET);
StrokeDetector detector;
bool oledReady=false,serialWasOpen=false;
unsigned long lastSampleMs=0,lastDisplayMs=0;
bool latestInRange=false;

bool buttonPressed() {
  static bool last=HIGH,stable=HIGH;
  static unsigned long changed=0;
  const bool reading=digitalRead(BUTTON_PIN);
  if(reading!=last){last=reading;changed=millis();}
  if(millis()-changed>=40 && reading!=stable){stable=reading;return stable==LOW;}
  return false;
}

bool acknowledges(uint8_t addr) {
  Wire.beginTransmission(addr);return Wire.endTransmission()==0;
}

void drawMeasurements(unsigned long now) {
  display.clearDisplay();display.setTextColor(SSD1306_WHITE);display.setTextSize(1);
  display.setCursor(0,0);display.print(F("MOTION TEST  #"));display.print(detector.count);
  display.setCursor(0,13);display.print(F("Down: "));
  if(!detector.stale(now)) {display.print(detector.downMm/10.0f,1);display.print(F(" cm"));}
  else display.print(F("--"));
  display.setCursor(0,25);display.print(F("Up:   "));
  if(detector.haveUp && !detector.stale(now)) {display.print(detector.upMm/10.0f,1);display.print(F(" cm"));}
  else display.print(F("--"));
  display.setCursor(0,37);display.print(F("Rate: "));
  if(!detector.stale(now) && detector.bpm>0){display.print(detector.bpm,0);display.print(F(" /min"));}
  else display.print(F("--"));
  display.setCursor(0,53);
  if(!latestInRange) display.print(F("CHECK TARGET / RANGE"));
  else if(detector.unsupportedMotion) display.print(F("MOTION OUT OF RANGE"));
  else if(detector.stale(now)) display.print(F("WAITING FOR STROKES"));
  else display.print(F("Button: reset count"));
  display.display();
}

void setup() {
  digitalWrite(BUZZER_PIN,LOW);pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(TRIG_PIN,LOW);pinMode(TRIG_PIN,OUTPUT);pinMode(ECHO_PIN,INPUT);
  pinMode(BUTTON_PIN,INPUT_PULLUP);Serial.begin(115200);
  const bool i2cReady=Wire.begin(OLED_SDA_PIN,OLED_SCL_PIN,I2C_CLOCK_HZ);
  Wire.setTimeOut(I2C_TIMEOUT_MS);
  if(i2cReady) {
    uint8_t address=OLED_ADDR;
    if(!acknowledges(address)) address=OLED_ALT_ADDR;
    if(acknowledges(address)) oledReady=display.begin(SSD1306_SWITCHCAPVCC,address,true,false);
  }
  if(oledReady) display.setTextWrap(false);
}

void loop() {
  if(buttonPressed()) detector.reset();
  const unsigned long now=millis();
  if(now-lastSampleMs<SAMPLE_INTERVAL_MS) return;
  lastSampleMs=now;
  digitalWrite(TRIG_PIN,LOW);delayMicroseconds(2);
  const unsigned long t=millis();
  digitalWrite(TRIG_PIN,HIGH);delayMicroseconds(10);digitalWrite(TRIG_PIN,LOW);
  const unsigned long echo=pulseIn(ECHO_PIN,HIGH,ECHO_TIMEOUT_US);
  const float gap=echo/5.8f;
  latestInRange=echo!=0 && gap>=MIN_GAP_MM && gap<=MAX_GAP_MM;
  const StrokeResult event=detector.add(t,gap,latestInRange);
  if(Serial) {
    if(!serialWasOpen) {
      if(!oledReady) Serial.println(F("# OLED unavailable; serial measurements continue. Check wiring/address."));
      Serial.println(F("timestamp_ms,gap_mm,in_range,event,down_mm,up_mm,bpm,hz,count"));
      serialWasOpen=true;
    }
    Serial.print(t);Serial.print(',');if(echo)Serial.print(gap,1);
    Serial.print(',');Serial.print(latestInRange?1:0);Serial.print(',');
    if(event.down)Serial.print(F("down"));else if(event.up)Serial.print(F("up"));
    Serial.print(',');if(!detector.stale(t))Serial.print(detector.downMm,1);
    Serial.print(',');if(detector.haveUp && !detector.stale(t))Serial.print(detector.upMm,1);
    Serial.print(',');if(detector.bpm>0 && !detector.stale(t))Serial.print(detector.bpm,1);
    Serial.print(',');if(detector.bpm>0 && !detector.stale(t))Serial.print(detector.bpm/60.0f,3);
    Serial.print(',');Serial.println(detector.count);
  } else serialWasOpen=false;
  if(oledReady && millis()-lastDisplayMs>=DISPLAY_INTERVAL_MS) {
    drawMeasurements(millis());lastDisplayMs=millis();
  }
}
