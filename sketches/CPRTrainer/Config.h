#pragma once

// GPIO numbers, not physical header positions.
#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 2
#define BUTTON_PIN 4
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_RESET -1
#define OLED_ADDR 0x3C
#define OLED_ALT_ADDR 0x3D
// Physical top yellow band on the two-color 128x64 OLED.
#define OLED_YELLOW_ROWS 16
#define SAMPLE_INTERVAL_MS 70UL
#define ECHO_TIMEOUT_US 30000UL
#define DISPLAY_INTERVAL_MS 210UL
#define BUTTON_DEBOUNCE_MS 40UL
#define COUNTDOWN_MS 5000UL
#define RESULTS_PAGE_MS 10000UL
#define METRONOME_BPM 110UL
#define CHIRP_US 50000ULL

// Adult compression targets; these are separate from detector acceptance limits.
#define DEPTH_LOW_MM 50.0f
#define DEPTH_HIGH_MM 60.0f
#define DEPTH_REFERENCE_MM 55.0f
#define RATE_LOW_BPM 100.0f
#define RATE_HIGH_BPM 120.0f
#define RATE_REFERENCE_BPM 110.0f
