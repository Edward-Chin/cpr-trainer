#pragma once
#include <stdint.h>

// Measurement acceptance limits, NOT CPR target ranges. Validate on the fixture.
#define MIN_GAP_MM 20.0f
#define MAX_GAP_MM 500.0f
#define REVERSAL_MM 8.0f
#define REVERSAL_CONFIRM_SAMPLES 2
#define MIN_TRAVEL_MM 10.0f
#define MAX_TRAVEL_MM 120.0f
#define MIN_PERIOD_MS 280UL
#define MAX_PERIOD_MS 2000UL
#define MAX_SAMPLE_GAP_MS 210UL
#define MAX_SAMPLE_STEP_MM 80.0f

struct StrokeResult {
  bool down = false;
  bool up = false;
  float downMm = 0;
  float upMm = 0;
  float bpm = 0;
  uint32_t troughMs = 0;
};

// Causal extrema tracker: no averaging that would flatten stroke peaks.
// A turning point is confirmed only after movement reverses by REVERSAL_MM.
class StrokeDetector {
 public:
  uint32_t count = 0;
  float downMm = 0, upMm = 0, bpm = 0;
  bool haveDown = false, haveUp = false;
  bool unsupportedMotion = false;
  uint32_t lastDownMs = 0;

  void reset() { clearTracking(); count = 0; }
  bool stale(uint32_t now) const {
    return !haveDown || uint32_t(now - lastDownMs) > MAX_PERIOD_MS;
  }

  StrokeResult add(uint32_t t, float d, bool valid) {
    StrokeResult result;
    if (!valid || !(d >= MIN_GAP_MM && d <= MAX_GAP_MM)) {
      clearTracking(); return result;
    }
    if (initialized && (uint32_t(t-lastSampleMs)>MAX_SAMPLE_GAP_MS ||
        absolute(d-lastDistance)>MAX_SAMPLE_STEP_MM)) {
      clearTracking(); // Never bridge missing samples or target jumps.
    }
    lastSampleMs=t; lastDistance=d;
    if (!initialized) {
      initialized=true; top=d; topMs=t; falling=false; return result;
    }
    if (haveDown && uint32_t(t-lastDownMs)>MAX_PERIOD_MS) {
      haveDown=haveUp=false; bpm=0; havePreviousTrough=false;periodCount=periodIndex=0;
    }
    if (!falling) {
      if (d>=top) {top=d;topMs=t;}
      if (top-d>=REVERSAL_MM) {
        if (haveBottom) {
          const float travel=top-bottom;
          if (travel>=MIN_TRAVEL_MM && travel<=MAX_TRAVEL_MM &&
              uint32_t(topMs-bottomMs)<=MAX_PERIOD_MS) {
            upMm=travel;haveUp=true;result.up=true;result.upMm=travel;
          } else {haveUp=false;}
        }
        bottom=d;bottomMs=t;falling=true;haveBottom=false;rejectedStroke=false;
      }
    } else {
      if (d<=bottom) {bottom=d;bottomMs=t;}
      if (top-bottom>MAX_TRAVEL_MM || uint32_t(t-topMs)>MAX_PERIOD_MS) {
        rejectedStroke=true;unsupportedMotion=true;
        haveDown=haveUp=havePreviousTrough=false;bpm=0;periodCount=periodIndex=0;
      }
      if (d-bottom>=REVERSAL_MM) {
        // Do not commit a stroke on a lone rebound just before target loss.
        // Keep the largest rebound value so confirmation does not lose a peak.
        if(riseCount==0 || d>reboundTop) {reboundTop=d;reboundTopMs=t;}
        if(++riseCount<REVERSAL_CONFIRM_SAMPLES) return result;
        const float travel=top-bottom;
        if (!rejectedStroke && travel>=MIN_TRAVEL_MM) {
          downMm=travel;haveDown=true;lastDownMs=bottomMs;count++;
          bpm=0;unsupportedMotion=false;
          if (havePreviousTrough) {
            const uint32_t period=bottomMs-previousTroughMs;
            if (period>=MIN_PERIOD_MS && period<=MAX_PERIOD_MS) {
              periods[periodIndex]=period;periodIndex=(periodIndex+1)%3;
              if(periodCount<3)periodCount++;
              uint32_t total=0;
              for(uint8_t i=0;i<periodCount;i++)total+=periods[i];
              bpm=60000.0f*periodCount/total;
            } else {unsupportedMotion=true;periodCount=periodIndex=0;}
          }
          previousTroughMs=bottomMs;havePreviousTrough=true;haveBottom=true;
          result.down=true;result.downMm=travel;result.bpm=bpm;result.troughMs=bottomMs;
        } else {havePreviousTrough=false;bpm=0;haveBottom=false;periodCount=periodIndex=0;}
        top=reboundTop;topMs=reboundTopMs;falling=false;riseCount=0;
      } else {riseCount=0;}
    }
    return result;
  }
 private:
  bool initialized=false, falling=false, haveBottom=false, havePreviousTrough=false;
  bool rejectedStroke=false;
  uint8_t riseCount=0;
  float reboundTop=0;
  uint32_t reboundTopMs=0;
  uint32_t periods[3]={0,0,0};
  uint8_t periodCount=0,periodIndex=0;
  float top=0,bottom=0,lastDistance=0;
  uint32_t topMs=0,bottomMs=0,lastSampleMs=0,previousTroughMs=0;
  static float absolute(float v) {return v<0?-v:v;}
  void clearTracking() {
    initialized=falling=haveBottom=havePreviousTrough=false;
    haveDown=haveUp=false;downMm=upMm=bpm=0;
    riseCount=0;
    rejectedStroke=unsupportedMotion=false;periodCount=periodIndex=0;
  }
};
