#ifndef ARDUINO
#include "StrokeDetector.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
  StrokeDetector d;
  // Stationary jitter must never become strokes.
  for(uint32_t t=0;t<10000;t+=70)d.add(t,150+3*std::sin(t),true);
  assert(d.count==0);
  // 5 cm at ~107/min: replay samples, without assumed CPR targets.
  d.reset();
  for(uint32_t t=0;t<10000;t+=70)d.add(t,125+25*std::cos(2*3.14159265*t/560),true);
  assert(d.count>=16 && d.count<=18);
  assert(std::fabs(d.downMm-50)<1 && std::fabs(d.bpm-60000.0/560)<1);
  // A pause makes old readings unavailable, and cannot add strokes.
  const auto count=d.count;
  for(uint32_t t=10000;t<14000;t+=70)d.add(t,150,true);
  assert(d.stale(14000) && d.bpm==0 && d.count<=count+1);
  // A new, offset baseline must give the same relative travel.
  d.reset();
  for(uint32_t t=0;t<10000;t+=70)d.add(t,325+25*std::cos(2*3.14159265*t/560),true);
  assert(std::fabs(d.downMm-50)<1);
  // Oversize 20 cm oscillations are rejected, not clipped to plausible depth.
  d.reset();
  for(uint32_t t=0;t<10000;t+=70)d.add(t,250+100*std::cos(2*3.14159265*t/840),true);
  assert(d.count==0);
  // Missing data clears phase/rate; reacquisition cannot join old extrema.
  d.add(11000,0,false); assert(d.stale(11000) && d.bpm==0);
  auto e=d.add(11070,200,true);assert(!e.down && !e.up);
  // Same for an excessive timestamp gap, with otherwise normal measurements.
  d.add(11140,170,true);e=d.add(12000,160,true);assert(!e.down && d.stale(12000));
  // Very fast synthetic motion should not produce an accepted 500/min rate.
  d.reset();
  for(uint32_t t=0;t<1500;t+=20)d.add(t,125+25*std::cos(2*3.14159265*t/120),true);
  assert(d.bpm==0 && d.unsupportedMotion);
  // A single rebound followed by target loss must not count as a stroke.
  d.reset();
  const float removal[]={150,150,132,125,115,162,1960};
  for(uint32_t i=0;i<7;i++)d.add(i*70,removal[i],removal[i]<=500);
  assert(d.count==0);
  std::cout<<"Passed stationary noise, motion, pause, baseline offset, oversized travel, dropout, gap and excessive-rate checks.\n";
}
#endif
