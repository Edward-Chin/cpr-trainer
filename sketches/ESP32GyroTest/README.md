# ESP32 gyro serial test

Assumes a classic ESP32 Dev Module and the **Adafruit L3GD20H** breakout.
This is a standalone wired sensor test; it does not implement wireless or angle integration.

| L3GD20H breakout | ESP32 |
| --- | --- |
| VIN | 3V3 |
| GND | GND |
| SDA | GPIO21 / D21 |
| SCL | GPIO22 / D22 |

Use soldered headers and a common ground. ESP32 GPIOs must not receive 5 V.

1. Connect the ESP32 by a USB data cable.
2. Run the Zed task **Arduino: List connected boards** and put its port in `arduino.env`.
3. Run **Arduino: Build and upload**, then open the serial monitor at 115200 baud.
4. Hold the sensor still, then rotate around each axis. Rates should be near zero
   when still (some bias is normal), and change sign when rotation reverses.

The sketch probes addresses 0x6A and 0x6B for WHO_AM_I 0xD7, reports angular
velocity in degrees/second at approximately 10 lines/second, and retries with an
I2C scan on failure. A device at 0x68 is not evidence of an L3GD20H.
The +/-245 degrees/second range can saturate during fast rotation.

The selected board is `esp32:esp32:esp32`. If the module is an ESP32-S3, C3, or
another variant, select its actual board and verify GPIO availability before upload.

To return to the main Pro Micro prototype, restore FQBN
`SparkFun:avr:promicro:cpu=16MHzatmega32U4`, sketch `sketches/StrokeTest`, and its USB port.
