#pragma once
#include <stdint.h>
#include <math.h>
#include "Config.h"
#include "StrokeDetector.h"

enum class Screen { Start, Countdown, Running, Results };

// One observation per completed downstroke, never per screen refresh.
// Welford's method avoids cancellation when computing variance.
struct MetricStats {
  uint32_t count = 0, low = 0, high = 0;
  float mean = 0, m2 = 0, absoluteErrorSum = 0;

  void add(float value, float lo, float hi, float reference) {
    ++count;
    const float delta = value - mean;
    mean += delta / count;
    m2 += delta * (value - mean);
    absoluteErrorSum += fabsf(value - reference);
    if (value < lo) ++low;
    else if (value > hi) ++high;
  }
  uint32_t inRange() const { return count - low - high; }
  float sd() const { return count ? sqrtf(fmaxf(0, m2 / count)) : 0; }
  float meanAbsoluteError() const { return count ? absoluteErrorSum / count : 0; }
};

struct SessionStats {
  MetricStats depth, rate;
  uint32_t pairedCount = 0, bothInRange = 0;

  void add(const StrokeResult &event) {
    if (!event.down) return;
    depth.add(event.downMm, DEPTH_LOW_MM, DEPTH_HIGH_MM, DEPTH_REFERENCE_MM);
    // These are rolling detector BPM estimates, not independent cycle rates.
    if (event.bpm <= 0) return;
    rate.add(event.bpm, RATE_LOW_BPM, RATE_HIGH_BPM, RATE_REFERENCE_BPM);
    ++pairedCount;
    if (event.downMm >= DEPTH_LOW_MM && event.downMm <= DEPTH_HIGH_MM &&
        event.bpm >= RATE_LOW_BPM && event.bpm <= RATE_HIGH_BPM) ++bothInRange;
  }
};

class Session {
 public:
  Screen screen = Screen::Start;
  uint32_t phaseStartMs = 0, sessionStartMs = 0, durationMs = 0;
  StrokeDetector detector;
  SessionStats stats;

  void press(uint32_t now) {
    if (screen == Screen::Start || screen == Screen::Results) {
      screen = Screen::Countdown;
      phaseStartMs = now;
    } else if (screen == Screen::Running) {
      durationMs = now - sessionStartMs;
      screen = Screen::Results;
      phaseStartMs = now;
    }
    // Countdown consumes presses, including a held button, without restarting.
  }

  void tick(uint32_t now) {
    if (screen == Screen::Countdown && uint32_t(now - phaseStartMs) >= COUNTDOWN_MS) {
      detector.reset();
      stats = SessionStats{};
      durationMs = 0;
      sessionStartMs = phaseStartMs = now;
      screen = Screen::Running;
    }
  }

  StrokeResult sample(uint32_t now, float gapMm, bool valid) {
    if (screen != Screen::Running) return StrokeResult{};
    const StrokeResult event = detector.add(now, gapMm, valid);
    stats.add(event);
    return event;
  }
  uint32_t elapsed(uint32_t now) const {
    return screen == Screen::Running ? now - sessionStartMs : durationMs;
  }
};
