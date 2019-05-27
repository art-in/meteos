#include "display.h"
#include "timings.h"
#include "utils.h"

RTC_DATA_ATTR int display_reading_idx = 0;
RTC_DATA_ATTR bool is_display_on = false;

Display* Display::instance = nullptr;

bool Display::is_on() { return is_display_on; }

void Display::init() {
  log_ln("display: init...", true);
  auto before_ms = millis();

  display.begin();
  display.setContrast(255);

  log_ln("display: init...done in " + String(millis() - before_ms) + "ms",
         true);
}

void Display::ensure_init() {
  if (!initiated) {
    init();
    initiated = true;
  }
}

void Display::draw_next_reading(const Sample& sample) {
  log_ln("display: draw next reading...");
  auto before_ms = millis();

  if (is_display_on) {
    display_reading_idx++;

    if (display_reading_idx > 4) {
      display_reading_idx = 0;
    }
  } else {
    display_reading_idx = 0;
  }

  is_display_on = true;

  if (display_reading_idx == 4) {
    clear();
  } else {
    String title;
    String value;
    String value_postfix;

    switch (display_reading_idx) {
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

    ensure_init();

    display.clearBuffer();
    display.setFontPosTop();

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(100, 33, value_postfix.c_str());

    display.sendBuffer();

    auto now_us = get_epoch_time_us();
    timings_next_display_clear_time_us = now_us + DISPLAY_DELAY_US;
  }

  log_ln("display: draw next reading...done in " +
         String(millis() - before_ms) + "ms");
}

void Display::draw_wait_config() {
  log_ln("display: draw wait config...");
  ensure_init();

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_t0_11_mf);
  display.drawStr(0, random(45), "waiting config...");

  display.sendBuffer();
  log_ln("display: draw wait config...done");
}

void Display::draw_log(String str) {
  ensure_init();

  display.clearBuffer();
  display.setFontPosTop();

  display.setFont(u8g2_font_5x7_mf);
  // randomize y-position to avoid OLED pixels burnout.
  display.drawStr(0, random(59), str.c_str());
  display.sendBuffer();
}

void Display::clear() {
  timings_from_boot_to_display_clear_duration_us = millis() * uS_TO_MS_FACTOR;

  is_display_on = false;

  ensure_init();

  display.clearBuffer();
  display.sendBuffer();

  log_ln("display: cleared", true);
}