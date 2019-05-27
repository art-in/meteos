#include <Arduino.h>

#include "display.h"
#include "sleep.h"
#include "timings.h"
#include "utils.h"

RTC_DATA_ATTR int wakeup_number = 0;
RTC_DATA_ATTR bool is_wakeup_goal_make_sample;  // otherwise to clear display

esp_sleep_wakeup_cause_t Sleep::on_wakeup() {
  auto now_us = get_epoch_time_us();

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    timings_wakeup_timer_mistake_us = now_us - timings_next_wakeup_time_us;
  }

  log_ln("--------");
  log_ln("sleep: wakeup number: " + String(wakeup_number));
  log("sleep: time: ");
  log(now_us / uS_TO_S_FACTOR);
  log("s (");
  log(now_us);
  log_ln("us)");

  print_wakeup_reason(wakeup_reason);
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    log_ln("sleep: wakeup time mistake: " +
           String(timings_wakeup_timer_mistake_us / uS_TO_MS_FACTOR) + "ms");
  }

  ++wakeup_number;

  return wakeup_reason;
}

void Sleep::print_wakeup_reason(esp_sleep_wakeup_cause_t wakeup_reason) {
  bool not_after_deep_sleep = false;
  String reason;

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      reason = "RTC_IO";
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      reason = "RTC_CNTL";
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      reason = "timer";
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      reason = "touchpad";
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      reason = "ULP program";
      break;
    default:
      not_after_deep_sleep = true;
  }

  if (not_after_deep_sleep) {
    log_ln("sleep: wakeup not after deep sleep", true);
  } else {
    log_ln("sleep: wakeup reason: " + reason, true);
  }
}

void Sleep::plan_deep_sleep() {
  log("sleep: timings_from_boot_to_sample_duration_us: ");
  log(timings_from_boot_to_sample_duration_us);
  log_ln("");

  auto sample_wakeup_time_us =
      timings_next_sample_time_us - timings_from_boot_to_sample_duration_us;
  auto clear_display_wakeup_time_us =
      timings_next_display_clear_time_us -
      timings_from_boot_to_display_clear_duration_us;

  log("sleep: sample_wakeup_time_us: ");
  log(sample_wakeup_time_us);
  log_ln("");

  log("sleep: clear_display_wakeup_time_us: ");
  log(clear_display_wakeup_time_us);
  log_ln("");

  // wakeup to take sample
  timings_next_wakeup_time_us = sample_wakeup_time_us;
  ::is_wakeup_goal_make_sample = true;

  // or wakeup to clear display
  if (Display::is_on() &&
      (clear_display_wakeup_time_us < timings_next_wakeup_time_us)) {
    timings_next_wakeup_time_us = clear_display_wakeup_time_us;
    ::is_wakeup_goal_make_sample = false;
  }

  log_ln("sleep: this boot duration: " + String(millis()) + "ms");

  log("sleep: next timer wakeup goal: ");
  if (::is_wakeup_goal_make_sample) {
    log_ln("make sample");
  } else {
    log_ln("clear display");
  }

  timings_next_wakeup_time_us -= timings_wakeup_timer_mistake_us;

  auto now_us = get_epoch_time_us();
  if (now_us > timings_next_wakeup_time_us) {
    // log_ln("sleep: restarting instead of deep sleep...");
    // ESP.restart();
    timings_next_wakeup_time_us = now_us + 1;
  }

  int time_to_sleep_us = timings_next_wakeup_time_us - now_us;
  deep_sleep(time_to_sleep_us);
}

void Sleep::deep_sleep(int us) {
  // using ext1 with bitmask instead of ext0, since ext1 does not require RTC
  // peripherals to be powered on. by default, ESP32 will automatically power
  // down the peripherals not needed by the wakeup source.
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  // also wakeup by timer
  esp_sleep_enable_timer_wakeup(us);

  Serial.flush();

  // disconnect wifi otherwise may not connect on next boot.
  RadioWiFi::disconnect();

  log_ln("sleep: sleep for " + String(us / uS_TO_MS_FACTOR) + "ms...", true);
  esp_deep_sleep_start();
}

bool Sleep::is_wakeup_goal_make_sample() {
  return ::is_wakeup_goal_make_sample;
}