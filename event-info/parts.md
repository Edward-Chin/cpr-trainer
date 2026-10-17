# Technothon parts inventory

This file is the single source of truth, consolidated from the official IEEE Technical Inventory (2026-2027) and the team's original parts notes.

## Availability and precedence

- Assume all parts listed by either source are available. An omission or a generic description in the official inventory does not invalidate a more specific item in the team notes.
- Where the sources actually conflict, the official inventory takes precedence. Its finite quantities replace the former unlimited-quantity assumption.
- Numeric quantities below are official inventory counts, not guaranteed allocations to one team. `Unspecified` means no quantity was provided; it does not mean unlimited.
- Kit descriptions include their piece counts separately from the inventory quantity. Do not multiply ambiguous counts without checking their units.
- Team-specified models may overlap the official inventory; do not add their counts or assume every unit in a generic category has that model.
- For generically named parts, assume the generous best case: if a specific part foreseeably falls within the category, assume it is available for brainstorming and ask the user to check the exact part when it matters.

## Microcontrollers & Dev Boards

| Part | Quantity |
| --- | ---: |
| ESP-32 | 42 |
| Arduino UNO R3 | 22 |
| Arduino Mega | 1 |
| Arduino Pro microcontroller | 2 |
| NUCLEO-C031C6 | 2 |
| raspberry pi pico | 9 |

## Cables & Adapters

| Part | Quantity |
| --- | ---: |
| Auxiliar Cable | 14 |
| USB to Ethernet | 26 |
| USB to microusb | 43 |
| USB to USB B | 17 |
| USB to USB C | 8 |
| Auxiliar breadboard converter | 24 |
| Auxiliar to USB C | 1 |
| Auxiliar to apple | 3 |
| usb a to car port | 3 |

## Power (Supplies, Batteries & Holders)

| Part | Quantity |
| --- | ---: |
| Breadboard PSU | 12 |
| 4 cell battery holder | 6 |
| 1 cell battery holder | 2 |
| 9v battery holder | 9 |
| 9V batteries | 21 |

## Breadboards

| Part | Quantity |
| --- | ---: |
| breadboard half sized | 59 |
| breadboard full sized | 4 |

## Displays, Audio & Output

| Part | Quantity |
| --- | ---: |
| Speakers | 11 |
| LED displays | 22 |
| OLED Displays | 16 |
| buzzer | 69 |

## Wireless & IR

| Part | Quantity |
| --- | ---: |
| bluetooth module | 5 |
| IR Remote | 9 |
| IR Reciever | 4 |

## Inputs (Buttons, Switches & Controls)

| Part | Quantity |
| --- | ---: |
| joystick | 18 |
| 4x4 button pad | 3 |
| Numpads | 7 |
| Gikfun 6x6x4 3mm Tactile buttons 50 pcs | 1 |
| 6A 125-250V switch | 5 |
| 6A 125V switch | 20 |
| Push Buttons | 48 |

## Motors, Drivers & Relays

| Part | Quantity |
| --- | ---: |
| Motors | 29 |
| Wheels | 26 |
| Disk Motor? | 1 |
| Motor driver Boards | 11 |
| Motor Driver Breakouts | 4 |
| SRD-05VDC-SL-C Relay | 1 |

## Sensors

| Part | Quantity |
| --- | ---: |
| Short Flex/Bend Sensor | 2 |
| Force Sensors | 2 |
| Sound Detectors | 55 |
| Touch Sensors | 15 |
| Gyroscope Sensors (2-Axis ?) | 12 |
| Adafruit L3GD20H 3-Axis Gyroscope | 6 |
| Adafruit 3-Axis Accelerometer LISD3H | 4 |
| Adafruit 3-Axis Accelerometer MMA8451 | 1 |
| AHT10 Temperature Humidity Sensors | 9 |
| Miscellaneous Temp Humidity Sensor | 1 |
| Hypersonic Sensors | 15 |
| Time of Flight Sensors | 16 |
| Motion Detector Sensor | 21 |
| Light Sensors | 10 |
| Heart Rate Sensor | 1 |

## Passive Components, LEDs & Headers

| Part | Quantity |
| --- | ---: |
| ALLECIN Ceramic Capacitor Kit 240 PCS from 10 PF to 10 uF, each 10 pieces | 2 |
| Bojack 600 PCS Ceramic Capacitor Kit from 10 pF to 100 nF | 1 |
| Bojack 1000 PCS Resistors | 2 |
| Plusivo High Qaulity Resistor Kit | 1 |
| CompStudio 900 PCS Resistor Kit | 1 |
| E-Projects 16 values (10 ohm - 1M Ohm | 400 |
| Dicuno 5mm Light emitting diodes kit | 4 |
| Header Pins 50 PCS | 1 |

## Mounting Hardware (Screws/Standoffs)

| Part | Quantity |
| --- | ---: |
| Hapric 260 pcs M5 | 1 |
| Hapric 600 pcs M3 | 1 |

## Additional parts specified in team notes

These entries have no separate official quantity or no unambiguous official match. They are available under the combined-source assumption, but may overlap generic entries above.

| Part | Quantity |
| --- | --- |
| Jumper wires: various colors; male-to-male, male-to-female, female-to-female; approximately 4 and 8 inches | Unspecified |
| HC-SR04 ultrasonic sensor | Unspecified |
| LCD display | Unspecified |
| LED matrix | Unspecified |
| Raspberry Pi Pico W | Unspecified |
| Pro Micro (written `pro M1cro` in team notes) | Unspecified |
| Plusivo three-pin transistors: BC337-25, BC327-25, 2N2222, 2N2907, 2N3904, 2N3906, A1015, C1815, S8050, S8550 | Unspecified |
| 555 timer | Unspecified |
| Op-amp | Unspecified |
| Inductor | Unspecified |
| Perfboard | Unspecified |
| Rubber bands | Unspecified |
| Multimeter | Unspecified |
| Scissors | Unspecified |

## Additional specifications from team notes

These details supplement generic official entries; they do not establish the model or specification of every unit in an official quantity.

- **Controllers:** Arduino Uno R3 and Elegoo Uno R3; Elegoo Mega2560 R3; generic STM32. The official inventory identifies 22 Uno R3 boards, 1 Mega, and 2 NUCLEO-C031C6 boards; no separate brand counts are supplied.
- **Breadboard:** 30 rows, two sides, five main columns per side. The official inventory distinguishes half-sized and full-sized breadboards.
- **OLED:** GoldenMorning GME12864-11/12/13 family.
- **Buzzer:** TMB12A05, 5 V, active.
- **Bluetooth:** BT-5.0-Audio.
- **LEDs:** various colors, 20 mA limit; forward voltages 2.0-2.2 V or 3.0-3.2 V depending on color.
- **Resistors and capacitors:** many values available; named official kits and ranges appear above.
- **Joystick:** game-controller-style joystick on a circuit board.
- **Buttons:** 3 mm tactile button listed in team notes; retain the official Gikfun description separately as written.
- **IR:** remote and corresponding receiver are available; official counts are 9 remotes and 4 receivers, not 9 complete pairs.
- **USB:** data cables with many connector types at each end; official cable/adapter counts appear above.
- **Switches:** generic switches in team notes are supplemented by the two official current/voltage-rated entries above.
- **Sensors:** the generic accelerometer, motion sensor, sound detector, and light sensor entries are represented by their official categories above; no extra quantities are implied.

## Unresolved source labels and count ambiguities

- `Adafruit 3-Axis Accelerometer LISD3H`: official model spelling preserved; verify the module marking before selecting a driver.
- `Hypersonic Sensors`: official wording preserved. Do not assume these 15 units are HC-SR04 sensors; HC-SR04 availability is independently supplied by team notes.
- `raspberry pi pico`: the official 9-board quantity does not specify W versus non-W. Pico W availability is independently supplied by team notes.
- `Arduino Pro microcontroller`: the official 2-board quantity does not identify the exact variant or establish a count of Pro Micro boards.
- `Gyroscope Sensors (2-Axis ?)` and `Disk Motor?`: uncertainty is present in the official descriptions.
- `E-Projects 16 values (10 ohm - 1M Ohm`: official description is incomplete and its quantity is 400; the counting unit is not explained.
- Auxiliary cable/converter entries retain the official wording; exact connectors and adapter functions are unspecified.
