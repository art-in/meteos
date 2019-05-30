#pragma once

#include "libs/U8g2_Arduino-2.25.10/src/U8g2lib.h"

#include "sample.h"

class Display {
 public:
  Display();
  void init();

  void draw_logo();
  void draw_next_reading(const Sample&);
  void draw_wait_config();
  void draw_log(String);
  void clear();

  bool is_on();
  static Display* get_instance() { return Display::instance; }

 private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

  static Display* instance;
  int sample_reading_idx = 0;
  bool is_on_ = false;
};