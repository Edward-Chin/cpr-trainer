# CPR Training Device — Current Specification

Updated October 16, 2026. Project selected; depth-sensing design remains undecided.

## Purpose

A breadboard prototype for practicing adult CPR compression motion, fitting the event’s **training** theme. It provides live feedback on repetition rate, compression travel, and changes in hand orientation. A pillow-based fixture demonstrates motion measurement; it does not reproduce human chest mechanics or certify CPR proficiency.

## Operation

1. The user assumes the starting hand position and presses a button.
2. A **3-second countdown** allows the user to hold still while the device establishes its orientation reference and estimates stationary gyroscope bias. The pad design also needs an unloaded distance reference.
3. The session begins: measurement, live feedback, and an audible metronome run together.

Session duration and stop/reset behavior remain to be decided.

## Planned feedback

- Current compression rate in compressions/minute (BPM), with **too slow / on pace / too fast** feedback.
- Last stroke’s measured travel, with **too shallow / within range / too deep** feedback.
- Hand-angle deviation from the starting orientation, with a directional correction cue when sufficiently reliable.
- OLED display and buzzer metronome; prioritize one correction at a time for readability.

Adult reference targets: **100–120 compressions/minute**, depth **at least 5 cm while avoiding depths greater than 6 cm**. The metronome setting is not yet selected. These targets apply to adults, not infants or children. [AHA adult guidelines](https://cpr.heart.org/en/resuscitation-science/cpr-and-ecc-guidelines/adult-basic-life-support).

## Depth sensing — decision pending

**Recommended first prototype: distance sensor + pad.** A stiff pressing pad rests on a pillow over a firm base. An overhang provides a flat target. An ultrasonic sensor fixed beside the pillow points upward at the overhang’s underside. Travel equals unloaded gap minus current gap. Pad bending, rocking, pillow recovery, sensor clearance, and unwanted echoes must be tested. A listed time-of-flight sensor is a possible fallback.

**Alternative: accelerometer + gyroscope + glove.** Sensors are mounted together on a rigid support securely attached to the glove. Depth is inferred from acceleration with orientation correction and drift handling. Portability is attractive, but accurate depth across different practice objects is unverified.

## Hand-angle measurement

Use bias-corrected gyroscope readings to estimate orientation change. Detect sustained angle deviation, not merely sudden angular-velocity spikes. Sensor mounting must track the hand; a sensor on the pad measures pad orientation instead. Feedback refers to the starting pose, not an independently established “ideal” CPR posture. Angle tolerances and drift compensation remain to be validated.

## Constraints and first validation

- Use available inventory components, with reasonable external parts/materials permitted. No water; returned club components must be protected. The Fab Lab power supply is unavailable during judging.
- Board and judging-power arrangement remain undecided.
- First validate the actual fixture and depth measurement against known travel, including repeated strokes near two per second and different hand placements.
- Depth error must be small relative to the 1 cm target interval before giving precise depth classifications. Invalid readings must not be treated as valid strokes or zero depth.
