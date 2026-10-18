# CPRTrainer: integrated ESP32 prototype

Build/upload with the existing Zed tasks; `arduino.env` selects this sketch.
Stop serial capture/monitor before uploading. No new libraries are needed beyond
the installed Adafruit SSD1306/GFX libraries and ESP32 Arduino core.

## Operation

1. Start screen waits for a debounced button press (GPIO 4 to GND).
2. A five-second positioning countdown runs. Button presses during it are ignored.
3. Session starts with fresh detector/statistics and a 110 BPM metronome.
4. Button ends the session, silences the buzzer and freezes statistics/duration.
5. Summary, depth and rate pages rotate every ten seconds. Button repeats the
   countdown, then starts a fresh session. No automatic session timeout.

The button requires release before another press; holding it cannot skip screens.
The 5 V active buzzer uses the existing 2N3904 driver, not direct GPIO power.
Its chirp is 50 ms. An ESP timer services it every 5 ms independently of sensor
and OLED blocking calls; hardware timing still needs confirmation.

## Files

- `CPRTrainer.ino`: setup and cooperative main loop.
- `Config.h`: pin `#define`s, timing and target settings.
- `Hardware.h/.cpp`: button debounce, HC-SR04 sampling, metronome, CSV.
- `Session.h`: state transitions and streaming statistics; desktop-testable.
- `Display.h/.cpp`: real OLED rendering, adapted from `tmp/cpr_trainer.ino`.
- `StrokeDetector.h`: byte-for-byte copy of the current StrokeTest detector.

The supplied draft remains as reference. The integrated version removes simulated
sensor readings, dummy calibration, automatic stop, gyro hooks and posture scores.
Live bars retain the draft's low/target/high zones with moving value pointers.
The separate **FUTURE LEAN** arrow is a static concept illustration: it does not
measure or recommend a direction. No gyroscope, ESP-NOW or angle integration runs.

## Wiring

| ESP32 GPIO/power | Connection |
| --- | --- |
| 5 | HC-SR04 TRIG |
| 18 | ECHO divider junction: ECHO -> 4.7 kOhm -> GPIO18 -> 6.8 kOhm -> GND |
| 2 | 680 Ohm -> 2N3904 base; retain 10 kOhm base-to-GND resistor |
| 4 | Button to GND; internal pull-up |
| 21 / 22 | OLED SDA / SCL |
| 3V3 | Compatible OLED VCC |
| Regulated 5 V | HC-SR04 VCC, buzzer positive |
| GND | Common ground for all wired components |

Transistor emitter to GND, collector to buzzer negative. Retain the flyback diode
across the magnetic buzzer (stripe to positive). Use only one power source: USB
with PSU positive output isolated, or verified regulated PSU 5 V into VIN with
USB disconnected. Do not connect raw 9 V to the shared 5 V rail or ESP32 3V3.

## Reading the screens

- Live depth is **downstroke pad travel in mm**, rate is the detector's rolling
  BPM. Count and elapsed seconds appear beside the future-feature icon.
- Missing target, unsupported motion or stale data clears live values/pointers;
  TARGET / RANGE / WAIT explains why. This hides stale values; detection thresholds
  and the existing spike sensitivity have not changed.
- Session duration includes pauses but excludes countdown. Count includes accepted
  downstrokes only, including those for which rate is unavailable.
- DEPTH IN: percent of accepted downstrokes between 50 and 60 mm inclusive.
- RATE IN: percent of available rolling BPM observations between 100 and 120.
- BOTH IN: percent meeting both targets among strokes with rate available. This
  is not time on target or a general CPR proficiency score. No observations = `--`.
- AVG: mean. SD: population standard deviation. OFFSET: mean minus the stated
  reference (55 mm or 110 BPM). MAE: mean absolute difference from that reference.
  Values use the page's units, not percent. LOW/HIGH/IN are observation percentages.
- Rate statistics summarize overlapping, smoothed detector estimates, not raw
  independent cycle periods. Unavailable rates are excluded, never scored as zero.
- Stats accumulate once per confirmed downstroke, not every repeated CSV/display
  reading. Statistics use constant memory; raw histories are stored only on the PC.

Adult target source: [American Heart Association](https://cpr.heart.org/resources/what-is-cpr):
100–120/min, at least 5 cm while avoiding excessive depth over 6 cm. The 55 mm
midpoint is our descriptive reference, not a separate AHA requirement. These two
targets do not cover all CPR guidelines. This pillow/plate prototype measures pad
motion, not validated human chest depth, release quality or clinical proficiency.

## CSV and testing

The existing Capture distance CSV task and viewer work unchanged. Raw samples
continue in every state with the original nine-column format; only Running feeds
stroke detection. State-change messages start with `#` and are ignored by capture.
Count resets at each session start. Result stats freeze while raw sampling continues.
Do not average repeated `down_mm`/`bpm` CSV values to reconstruct session stats;
select `event=down` rows. Diagnostic metric fields may be blank when motion is
unsupported even if the unchanged detector accepted a depth event.

Desktop checks (outside the firmware; no fake-data mode in the sketch):

```
c++ -std=c++11 tests/cpr_session_checks.cpp -o /tmp/cpr-session-checks
/tmp/cpr-session-checks
```

Hardware acceptance before judging:

1. Confirm start screen, one press -> 5,4,3,2,1 -> session; held button does not end it.
2. Confirm 110 BPM chirps, real depth/rate updates and clearing on target removal.
3. Press to end: immediate silence, frozen count/duration, three pages at 10 seconds.
4. Restart; verify a new countdown and cleared results. End with zero strokes:
   no fabricated averages or percentages should appear.
5. Repeat with the actual battery/PSU and secured pillow/plate fixture.

Compilation and desktop checks do not establish physical OLED, sound or sensor
correctness. Existing echo glitches can still affect accepted counts/statistics.

Validated on October 18: ESP32 core 3.3.2 compile succeeded (326,399 bytes flash,
22,544 bytes static RAM); session/statistics desktop checks passed; detector copy
matches StrokeTest exactly. This integrated sketch has not been uploaded or
physically tested by the assistant.
