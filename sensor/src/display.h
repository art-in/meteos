#pragma once

#include "libs/U8g2_Arduino-2.25.10/src/U8g2lib.h"

#include "sample.h"

enum DisplayedReading {
  NONE = 0,
  TEMPERATURE = 1,
  HUMIDITY = 2,
  PRESSURE = 3,
  CO2 = 4,
  FIRST = TEMPERATURE,
  LAST = CO2
};

class Display {
 public:
  Display();
  void init();

  void draw_logo();
  void draw_next_reading(const Sample&);
  void draw_wait_config();
  void draw_log(String);
  void clear();

  bool is_reading_shown();
  static Display* get_instance() { return Display::instance; }

 private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

  static Display* instance;
  DisplayedReading reading = NONE;
  bool is_reading_shown_ = false;
};
