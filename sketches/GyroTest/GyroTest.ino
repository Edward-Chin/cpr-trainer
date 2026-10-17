#include <Wire.h>

// Adafruit L3GD20H breakout -> 5 V Pro Micro:
// VIN -> VCC, GND -> GND, SDA -> 2, SCL -> 3.
// Uses built-in Wire only. Values are angular velocity, not orientation.
uint8_t gyroAddress = 0;

bool readRegisters(uint8_t address, uint8_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(address, count) != count) {
    while (Wire.available()) Wire.read();
    return false;
  }
  for (uint8_t i = 0; i < count; ++i) data[i] = Wire.read();
  return true;
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(gyroAddress);
  Wire.write(reg);
  Wire.write(value);
  const uint8_t result = Wire.endTransmission();
  if (result != 0) {
    Serial.print(F("CONFIG WRITE FAILED at register 0x"));
    Serial.print(reg, HEX);
    Serial.print(F("; I2C error="));
    Serial.println(result);
  }
  return result == 0;
}

void scanBus() {
  Serial.println(F("I2C scan (2=address NACK, 4=bus error, 5=timeout):"));
  uint8_t found = 0;
  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    Wire.beginTransmission(address);
    const uint8_t result = Wire.endTransmission();
    if (result == 0) {
      ++found;
      Serial.print(F("  Device ACK at 0x"));
      Serial.println(address, HEX);
    } else if (result != 2) {
      Serial.print(F("  Bus error="));
      Serial.println(result);
      break;
    }
  }
  if (!found) Serial.println(F("  No I2C devices acknowledged."));
}

bool initializeGyro() {
  gyroAddress = 0;
  for (uint8_t address = 0x6A; address <= 0x6B; ++address) {
    uint8_t id = 0;
    const bool readOk = readRegisters(address, 0x0F, &id, 1);
    Serial.print(F("Probe 0x"));
    Serial.print(address, HEX);
    if (!readOk) {
      Serial.println(F(": register read failed"));
    } else {
      Serial.print(F(": WHO_AM_I=0x"));
      Serial.print(id, HEX);
      Serial.println(F(" (expected D7; D4 is older L3GD20)"));
    }
    if (readOk && id == 0xD7) {
      gyroAddress = address;
      break;
    }
  }
  if (!gyroAddress) return false;
  // Power down while configuring; disable FIFO and high-pass filtering.
  // Block-data-update keeps each axis's low/high bytes consistent.
  if (!writeRegister(0x20, 0x00) || !writeRegister(0x21, 0x00) ||
      !writeRegister(0x23, 0x80) || !writeRegister(0x24, 0x00) ||
      !writeRegister(0x39, 0x00) || !writeRegister(0x20, 0x0F)) {
    gyroAddress = 0;
    return false;
  }
  delay(250);
  Serial.print(F("L3GD20H detected at 0x"));
  Serial.println(gyroAddress, HEX);
  Serial.println(F("X/Y/Z in deg/s; uncalibrated, +/-245 deg/s range"));
  return true;
}

void setup() {
  Serial.begin(115200);
  const unsigned long started = millis();
  while (!Serial && millis() - started < 3000UL) {}
  Wire.begin();
  Wire.setClock(100000);
  Wire.setWireTimeout(25000, true);
}

void loop() {
  if (!gyroAddress && !initializeGyro()) {
    Serial.println(F("L3GD20H NOT FOUND / INIT FAILED: check power, SDA=2, SCL=3"));
    scanBus();
    delay(3000);
    return;
  }
  uint8_t data[6];
  // Bit 7 enables register auto-increment, starting at OUT_X_L.
  if (!readRegisters(gyroAddress, 0x28 | 0x80, data, sizeof(data))) {
    Serial.println(F("I2C READ FAILED; retrying detection"));
    gyroAddress = 0;
    delay(1000);
    return;
  }
  Serial.print(millis());
  Serial.print(F(" ms | "));
  for (uint8_t axis = 0; axis < 3; ++axis) {
    const uint8_t offset = axis * 2;
    const int16_t raw = (int16_t)((uint16_t)data[offset] |
                                ((uint16_t)data[offset + 1] << 8));
    Serial.print((char)('X' + axis));
    Serial.print(F("="));
    Serial.print(raw * 0.00875f, 2);
    Serial.print(F(" deg/s "));
  }
  Serial.println();
  delay(100);
}
