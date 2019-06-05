#include "display.h"
#include "utils.h"

Display* Display::instance = nullptr;

Display::Display() : display{U8G2_R0} { Display::instance = this; }

void Display::init() {
  METEOS_SCOPED_LOGGER("display: init");

  display.begin();
  display.setContrast(255);
}

bool Display::is_reading_shown() { return is_reading_shown_; }

void Display::draw_logo() {
  METEOS_SCOPED_LOGGER("display: draw logo");

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_inb19_mf);
  display.drawStr(15, 20, "meteos");

  display.sendBuffer();
}

void Display::draw_next_reading(const Sample& sample) {
  METEOS_SCOPED_LOGGER("display: draw next reading");

  if (is_reading_shown_) {
    sample_reading_idx++;

    if (sample_reading_idx > 4) {
      sample_reading_idx = 0;
    }
  } else {
    sample_reading_idx = 0;
  }

  if (sample_reading_idx == 4) {
    clear();
  } else {
    String title;
    String value;
    String value_postfix;

    is_reading_shown_ = true;

    switch (sample_reading_idx) {
      case 0:
        title = "temperature";
        value = String(sample.temperature);
        value_postfix = "C";
        break;
      case 1:
        title = "humidity";
        value = String(sample.humidity);
        value_postfix = "%";
        break;
      case 2:
        title = "pressure";
        value = String(sample.pressure);
        value_postfix = "mm";
        break;
      case 3:
        title = "co2";
        value = String(sample.co2);
        value_postfix = "ppm";
        break;
    }

    display.clearBuffer();
    display.setFontPosTop();

    // TODO: update fonts (profont)
    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(100, 33, value_postfix.c_str());

    display.sendBuffer();
  }
}

void Display::draw_wait_config() {
  METEOS_SCOPED_LOGGER("display: draw wait config");

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_profont11_mf);
  int pos_y = random(39);
  display.drawStr(0, pos_y, "waiting config...");
  display.drawStr(0, pos_y + 14, "(press to exit)");

  display.sendBuffer();
}

void Display::draw_log(String str) {
  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_5x7_mf);

  // randomize y-position to avoid OLED pixels burnout.
  display.drawStr(0, random(59), str.c_str());
  display.sendBuffer();
}

void Display::clear() {
  is_reading_shown_ = false;

  display.clearBuffer();
  display.sendBuffer();

  METEOS_LOG_LN("display: cleared");
}