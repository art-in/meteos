#include "display.h"
#include "utils.h"

Display* Display::instance = nullptr;

Display::Display() : display{U8G2_R0} { Display::instance = this; }

void Display::init() {
  log_ln("display: init...", true);
  auto before_ms = millis();

  display.begin();
  display.setContrast(255);

  log_ln("display: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

bool Display::is_on() { return is_on_; }

void Display::draw_logo() {
  log_ln("display: draw logo...");
  auto before_ms = millis();

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_inb19_mf);
  display.drawStr(15, 20, "meteos");

  display.sendBuffer();

  log_ln("display: draw logo...done in " + String(millis() - before_ms) + "ms");
}

void Display::draw_next_reading(const Sample& sample) {
  log_ln("display: draw next reading...");
  auto before_ms = millis();

  if (is_on_) {
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

    is_on_ = true;

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
        value_postfix = "ppx";
        break;
    }

    display.clearBuffer();
    display.setFontPosTop();

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(100, 33, value_postfix.c_str());

    display.sendBuffer();
  }

  log_ln("display: draw next reading...done in " +
         String(millis() - before_ms) + "ms");
}

void Display::draw_wait_config() {
  log_ln("display: draw wait config...");

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_t0_11_mf);
  display.drawStr(0, random(45), "waiting config...");

  display.sendBuffer();
  log_ln("display: draw wait config...done");
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
  is_on_ = false;

  display.clearBuffer();
  display.sendBuffer();

  log_ln("display: cleared", true);
}