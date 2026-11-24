# CPR Trainer

This project won 2nd place at the Technothon event hosted by Texas A&M University's IEEE Club.

[Devpost](https://ieee-hackathon.devpost.com/)

## How to Use the Device
Press the button to start a five-second countdown. A buzzer sets a 110 BPM pace while an OLED shows measured depth, rate, and elapsed time. Moving pointers show how the measurements compare with the configured targets of 50–60 mm and 100–120 compressions per minute.
Press again to finish. Three results pages rotate every ten seconds. The first shows the percentage of measured strokes meeting each target and both together. The next two show average rate and depth and the percentages above and below each range. Press again for a new session.

## Competition
Edward Chin, Seth Schumacher, Erikson Tran, and Nang Vu (Team Breathtakers) built and demonstrated the prototype at Texas A&M on October 16–18, 2026. The “Engineering for Disasters” challenge called for a project related to training, prevention, or response. We chose training and used club-provided electronics, a breadboard, and a pillow-and-lid fixture for our five-minute demonstration to four judges. Battery power allowed the demonstration to run without a bench supply.

Depth represents lid travel at the overhang. Tilting or flexing the lid can change that reading, and missed echoes can interrupt feedback. The device has not been clinically validated.

## Build and run

Use a classic ESP32 Dev Module, HC-SR04, 128×64 SSD1306 I²C OLED, push-button, and 5 V active buzzer. Follow the [wiring table and firmware guide](sketches/CPRTrainer/README.md#wiring), including the voltage divider on ECHO and transistor driver for the buzzer.

With Arduino CLI installed and the Espressif ESP32 board package available (tested with core 3.3.2), run these commands from the repository root:

```sh
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library" "Adafruit BusIO"
cp arduino.env.example arduino.env
bash scripts/arduino.sh build
bash scripts/arduino.sh boards
```

Set `ARDUINO_PORT` in your local `arduino.env` to the port reported for your board, close any serial monitor, then run `bash scripts/arduino.sh upload`. The included Zed tasks call the same scripts.

## Testing and data

To record practice movements, run `bash scripts/arduino.sh capture-distance` with Python 3 installed. Stop with Ctrl+C, open [the graph viewer](scripts/view-distance.html) locally in a browser, and select the CSV from `logs/`. The trace shows what the sensor saw during a session, including interruptions.

To check session transitions and statistics on a computer:

```sh
c++ -std=c++11 tests/cpr_session_checks.cpp -o /tmp/cpr-session-checks
/tmp/cpr-session-checks
```

[MIT license](LICENSE)
