#pragma once
#include <Arduino.h>
#include "Session.h"

struct DistanceSample {
  uint32_t timestampMs = 0, echoUs = 0;
  float gapMm = 0;
  bool valid = false;
};

void beginHardware();
bool buttonPressed();
bool readDistance(DistanceSample &sample);
void setMetronome(bool enabled);
void logSample(const DistanceSample &sample, const StrokeResult &event, const Session &session);
