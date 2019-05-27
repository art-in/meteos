#pragma once

#include "libs/U8g2_Arduino-2.25.10/src/U8g2lib.h"

#include "sample.h"

constexpr unsigned long DISPLAY_DELAY_US = 5000000;  // us

class Display {
 public:
  Display() : display{U8G2_R0} { Display::instance = this; }

  void draw_next_reading(const Sample&);
  void draw_wait_config();
  void draw_log(String);
  void clear();

  static bool is_on();
  static Display* get_instance() { return Display::instance; }

 private:
  static Display* instance;
  bool initiated = false;
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

  void init();
  void ensure_init();
};