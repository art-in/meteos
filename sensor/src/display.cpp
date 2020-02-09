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
    reading = DisplayedReading((reading + 1) % (LAST + 1));
  } else {
    reading = FIRST;
  }

  if (reading == NONE) {
    clear();
  } else {
    String title;
    String value;
    String value_units;

    is_reading_shown_ = true;

    switch (reading) {
      case TEMPERATURE:
        title = "temperature";
        value = String(sample.temperature);
        value_units = "C";
        break;
      case HUMIDITY:
        title = "humidity";
        value = String(sample.humidity);
        value_units = "%";
        break;
      case PRESSURE:
        title = "pressure";
        value = String(sample.pressure);
        value_units = "mm";
        break;
      case CO2:
        title = "co2";
        value = String(sample.co2);
        value_units = "ppm";
        break;
    }

    display.clearBuffer();
    display.setFontPosTop();

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());
    display.drawStr(100, 33, value_units.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

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

  display.setFont(u8g2_font_profont10_mf);

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