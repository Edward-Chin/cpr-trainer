# Distance capture

HC-SR04: TRIG -> 9, ECHO -> 10, VCC -> 5 V VCC, GND -> GND.
Pin constants use `#define`. Pin 4 holds the existing buzzer driver off.
The OLED and button are not used in this standalone sketch.

1. Select `sketches/DistanceCapture` in `arduino.env` (already selected).
2. Close any serial monitor and run Zed's **Arduino: Build and upload**.
3. Run **Arduino: Capture distance CSV (Ctrl+C to stop)**. Recording starts
   immediately when the serial connection opens; no button press is required.
4. Exercise the actual fixture, then press Ctrl+C. The task prints the saved
   `logs/distance-<date>-<time>.csv` path. Each run creates a new file.
5. Run **Arduino: View distance CSV graph** and select that CSV. Enter start/end
   times to inspect individual strokes; hover to inspect measurements.

The Mac saves the file; the microcontroller does not have file storage attached.
Python 3 and the existing Arduino CLI are used; no extra Python packages are needed.
The graph is an offline HTML file with no external dependencies.

CSV columns: `timestamp_ms,echo_duration_us,distance_mm,valid`.
Timestamp is microcontroller uptime at trigger. Samples are scheduled at least
70 ms apart (~14.3 Hz); actual timestamps capture delays. Timeout is 30 ms.
Distances use the same nominal conversion as DistanceTest (echo_us / 5.8 in mm).
No filtering is applied. Timeout: duration 0, blank distance, valid 0. Echoes
outside 20–4000 mm retain their calculated distance but have valid 0.
A valid flag does not prove that the echo came from the intended target.
One decimal place in the CSV does not imply 0.1 mm measurement accuracy.

The viewer uses elapsed time from the first saved sample, leaves gaps for invalid
readings and intervals over 210 ms, and flags timestamp resets rather than joining
unrelated sessions. Keep the board powered during each recording.

For reproducibility, note the fixture setup, measured travel, hand placement and
intended pace separately. Keep static checks and different motion trials in
separate recordings. Judge depth accuracy against a ruler/physical stops.

Repository note: `scripts/`, `.zed/`, and `logs/` are currently gitignored.
To share the workflow with teammates, also share `scripts/capture-distance.py`,
`scripts/view-distance.html`, the modified `scripts/arduino.sh`, and `.zed/tasks.json`.
