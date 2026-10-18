#include "../sketches/CPRTrainer/Session.h"
#include <cassert>
#include <cmath>
#include <iostream>

static void close(float actual, float expected) {
  assert(std::fabs(actual - expected) < 0.01f);
}

int main() {
  MetricStats metric;
  for (float d : {40.f, 50.f, 55.f, 60.f, 70.f}) metric.add(d, 50, 60, 55);
  assert(metric.count == 5 && metric.low == 1 && metric.high == 1 && metric.inRange() == 3);
  close(metric.mean, 55); close(metric.sd(), 10); close(metric.meanAbsoluteError(), 8);

  SessionStats summary;
  StrokeResult event;
  event.up = true; event.upMm = 55;
  summary.add(event); assert(summary.depth.count == 0);
  event.up = false; event.down = true; event.downMm = 50; event.bpm = 0;
  summary.add(event);
  assert(summary.depth.count == 1 && summary.rate.count == 0 && summary.pairedCount == 0);
  event.bpm = 100; summary.add(event);
  event.downMm = 60; event.bpm = 120; summary.add(event);
  event.downMm = 70; event.bpm = 130; summary.add(event);
  assert(summary.pairedCount == 3 && summary.bothInRange == 2);
  assert(summary.rate.count == 3 && summary.rate.high == 1);

  Session s;
  assert(s.screen == Screen::Start);
  s.sample(0, 100, true); assert(s.detector.count == 0);
  s.press(100); assert(s.screen == Screen::Countdown);
  s.press(200); assert(s.phaseStartMs == 100);
  s.tick(5099); assert(s.screen == Screen::Countdown);
  s.tick(5100); assert(s.screen == Screen::Running);
  for (uint32_t dt = 0; dt < 10000; dt += 70)
    s.sample(5100 + dt, 125 + 25 * std::cos(2 * 3.14159265 * dt / 560), true);
  assert(s.stats.depth.count >= 16 && s.stats.depth.count <= 18);
  assert(s.stats.depth.count == s.detector.count);
  assert(s.stats.rate.count == s.stats.depth.count - 1);
  const auto count = s.stats.depth.count;
  s.tick(20000); assert(s.screen == Screen::Running); // no sample-code auto-stop
  assert(s.detector.stale(20000));
  s.press(20000); assert(s.screen == Screen::Results && s.durationMs == 14900);
  for (uint32_t dt = 0; dt < 3000; dt += 70) s.sample(20100 + dt, 100, true);
  assert(s.stats.depth.count == count && s.elapsed(30000) == 14900);
  s.press(30000); assert(s.screen == Screen::Countdown);
  s.tick(34999); assert(s.stats.depth.count == count);
  s.tick(35000);
  assert(s.screen == Screen::Running && s.stats.depth.count == 0 && s.detector.count == 0);
  s.press(35001); assert(s.stats.rate.count == 0 && s.durationMs == 1);

  // Countdown and duration use rollover-safe elapsed arithmetic.
  Session wrap;
  const uint32_t start = UINT32_MAX - 1000;
  wrap.press(start); wrap.tick(uint32_t(start + 4999));
  assert(wrap.screen == Screen::Countdown);
  wrap.tick(uint32_t(start + 5000)); assert(wrap.screen == Screen::Running);
  wrap.press(uint32_t(start + 6000)); assert(wrap.durationMs == 1000);
  std::cout << "Passed session transitions, countdown, restart, freeze, rollover, stroke-only stats, missing rate, and target boundaries.\n";
}
