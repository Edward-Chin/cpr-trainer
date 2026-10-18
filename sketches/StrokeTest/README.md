# StrokeTest — first live measurement prototype

The SSD1306 OLED shows last downstroke travel, last upstroke travel, rate per
minute, stroke count and a status message. Layout is intentionally provisional.
Serial output additionally includes frequency in Hz and raw gap.
No clinical scoring, metronome, gyro, unloaded-depth calibration or angle feedback
is implemented. The existing buzzer driver is held off.

## Run

Set `ARDUINO_SKETCH="sketches/StrokeTest"` in `arduino.env`, then use the existing
Zed Build and upload task. Runs immediately, with or without a serial monitor.
This sketch now targets the classic ESP32 Dev Module (`esp32:esp32:esp32`).
GPIO 4 button to GND resets tracking/count; it is not a start/pause button here.
OLED SDA=21, SCL=22; HC-SR04 TRIG=5, ECHO=18; buzzer transistor driver=2.
Numbers are GPIO numbers, not header positions. The ESP32 Wire API explicitly
selects SDA/SCL and uses a 25 ms transaction timeout.
Pin definitions use `#define`. OLED address 0x3C or 0x3D is detected at startup.
If the OLED is absent, serial measurements still run. The module must be connected
before startup. Use Serial monitor (or monitor and save log) to inspect output.
The Capture distance CSV task now recognizes both sketches and preserves all
columns. The graph viewer plots either format and shows stroke metrics on hover.
DistanceCapture additionally records raw echo duration; StrokeTest records gap
and stroke metrics. Restart the capture task after changing sketches.

## ESP32 wiring and power

- Supply the HC-SR04 with regulated 5 V. Connect all wired circuit grounds.
- ECHO must pass through a voltage divider: ECHO -> 4.7 kOhm -> GPIO 18,
  with 6.8 kOhm from GPIO 18 to GND (about 2.96 V for a 5 V echo).
  Alternatively use 2.2 kOhm and 3.3 kOhm respectively (3.0 V).
- Keep OLED SDA/SCL pull-ups at 3.3 V. Power a compatible OLED from 3.3 V;
  a display that accepts 5 V power does not necessarily level-shift its I2C pins.
- Leave the buzzer disconnected for this measurement test, or use its transistor
  driver: buzzer + to 5 V, buzzer - to NPN collector, emitter to GND,
  GPIO 2 through 680 Ohm to base, 10 kOhm from base to emitter. Verify the actual
  transistor pinout. For the magnetic buzzer, add a flyback diode across it,
  cathode/stripe to +5 V and anode to collector. The sketch holds GPIO 2 LOW.
- GPIO 2 and 5 are ESP32 strapping pins. Do not add external circuitry that
  forces inappropriate levels at reset; disconnect attached loads if boot/upload
  fails. The specified low-side NPN driver pulls GPIO 2 toward GND.
- Verify PSU rail voltage and polarity with a meter. Never feed 5 V or 9 V into
  ESP32 3V3. Confirm the development board's VIN/5V input before supplying it.
  Avoid connecting external power to that input while USB also powers the board
  unless its power-isolation circuit has been verified.

The photographed MB102-style PSU uses linear regulators, not a switching buck
converter. Test the final battery-powered assembly under load; a 9 V rectangular
battery and regulator heating may limit runtime or cause resets, especially once
wireless communication is enabled.

## Algorithm and configurable limits

`StrokeDetector.h` contains the shared detection algorithm. A fixed sensor points
at the moving pad: decreasing gap is down, increasing gap is up. Extrema are
confirmed after an 8 mm reversal, suppressing small fluctuations without averaging
away peaks. Downstroke confirmation requires two consecutive valid samples at
least 8 mm above the trough. This adds about one sampling interval (~70 ms) of
confirmation delay and rejects a lone rebound immediately before target loss. A downstroke is reported on confirmed upward reversal; an upstroke on
confirmed downward reversal. They therefore update at different times.

Travel is the difference between neighboring turning points. No fixed baseline or
expected amplitude/frequency is used. This does NOT measure incomplete release
relative to the original unloaded pad position; that requires a separate reference.
Partial first strokes and repositioning can still be mistaken for real strokes.

Prototype limits (acceptance limits, not correct-CPR targets):

- Sensor-to-target gap: 20–500 mm, configurable for the actual fixture.
- Reportable travel: 10–120 mm. Smaller movement is suppressed; oversized downward
  excursions are rejected, not clipped. Confirm performance near the minimum.
- Accepted cycle period: 280–2000 ms (about 30–214/min). Rates outside that range
  remain unavailable and show a motion-range message. Extremely fast real motion
  can alias at this sample rate; rejection of calculated rates cannot prevent that.
- Rate uses elapsed time across up to the last three accepted trough-to-trough
  intervals. It becomes available after two troughs. Hz = rate / 60.
- No confirmed downstroke for 2 seconds: hide old travel/rate and wait for strokes.
  This intentionally does not measure arbitrarily slow movement.
- Invalid range/echo, a sample gap over 210 ms or a single step over 80 mm resets
  tracking. Count is preserved until the button reset. Sample step rejection is a
  heuristic and needs testing with real abrupt motion and target removal.

Measurements are scheduled at least 70 ms apart. OLED refresh is approximately
210 ms; actual serial timestamps reveal any additional delay. Sampled extrema can
underestimate true travel, and peak confirmation adds delay. Values displayed to
0.1 cm are not a claim of that accuracy. All limits need further fixture validation.

## Validation so far

The ESP32 port compiles with Espressif Arduino core 3.3.2: 322,271 bytes of flash
and 22,432 bytes of static RAM. Existing desktop detector checks pass unchanged.
This port has not been uploaded or physically verified by the assistant.

The previous version compiled for the 5 V/16 MHz Pro Micro. The exact C++ detector was replayed against
`logs/distance-20261017-200314-000342.csv`: 27 troughs between elapsed 30–45 seconds,
median downward travel 80.5 mm. These are algorithm outputs, not ground truth.
Replay results are saved at `logs/strokes-replay-20261017-200314.csv`.

Desktop synthetic checks cover stationary jitter, known periodic travel, pauses,
baseline offsets, oversized movement, missing echoes, sampling gaps and an
excessive calculated frequency. Build/run with a C++11 compiler:

```
c++ -std=c++11 sketches/StrokeTest/detector_checks.cpp -o /tmp/stroke-checks
/tmp/stroke-checks
c++ -std=c++11 sketches/StrokeTest/replay.cpp -o /tmp/stroke-replay
/tmp/stroke-replay < logs/<raw-distance-recording>.csv
```

Both desktop entry points are excluded during Arduino compilation.
Physical OLED rendering and live measurement with this sketch remain untested.

## Next recordings

Use separate raw CSVs for:
1. Stationary holds at several fixture heights (10–20 seconds each).
2. Travel constrained/measured at 3, 5 and 8 cm, with counted repetitions.
3. Slow and fast strokes, irregular timing, pauses at top and bottom.
4. Partial release, changed hand positions, another operator.
5. Deliberate target removal/re-entry and fixture bumps.

Use a ruler/stops or a side video as independent evidence, rather than assuming
a metronome setting or intended depth equals the motion that occurred. Validate
on recordings not used to choose the thresholds. If these fail, revise mechanics
or sensing before adding stronger feedback claims.

## Target-removal regression (October 17)

The step-and-hold recording `distance-20261017-205816-605025.csv` originally
produced one false stroke during target removal and no rate estimates. Requiring
a second confirmation sample removes that event on replay (zero strokes), while
the earlier 30–45 second cyclic recording still yields 27 strokes with median
80.5 mm travel. This is evidence for the specific fix, not universal immunity to
removal artifacts. Replay accepts both raw and StrokeTest CSV schemas.

Long move-and-hold tests validate measured levels, not rhythmic stroke detection:
this detector waits for reversal and rejects phases lasting over two seconds.
Two successive downward steps with a hold are not two complete compressions.
To isolate sensor variation, rigidly support the target instead of holding it.
