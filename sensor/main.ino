#include <Arduino.h>
#include <functional>

#include "src/config.h"
#include "src/display.h"
#include "src/radio-bt.h"
#include "src/radio-wifi.h"
#include "src/sample.h"
#include "src/sensors.h"
#include "src/sleep.h"
#include "src/utils.h"

void setup() {
  Serial.begin(115200);

  Sleep sleep;
  auto wakeup_reason = sleep.on_wakeup();

  bool is_wakeup_by_button = wakeup_reason == ESP_SLEEP_WAKEUP_EXT1;
  bool is_wakeup_by_timer = wakeup_reason == ESP_SLEEP_WAKEUP_TIMER;
  bool is_initial_boot = !is_wakeup_by_button && !is_wakeup_by_timer;

  Display display;
  Sensors sensors;
  Config config;
  config.init();

  if (is_initial_boot) {
    // use initial boot as only time to update wifi config
    config.clear();

    // TODO: show draw logo
    log_ln("main: initial boot.", true);
    delay(1000);

    if (!config.has_wifi_config()) {
      RadioBT bt{config};
      bt.start_advertising();

      config.wait_wifi_config([&display]() { display.draw_wait_config(); });
      display.clear();
    }

  } else if (is_wakeup_by_button) {
    auto last_sample = Sensors::get_last_sample();
    display.draw_next_reading(last_sample);

  } else if (is_wakeup_by_timer) {
    if (Sleep::is_wakeup_goal_make_sample()) {
      // wakeup goal - make sample

      sensors.init(is_initial_boot);
      auto sample = sensors.take_sample();

      RadioWiFi wifi{config.wifi_ssid, config.wifi_pass};
      if (wifi.connect()) {
        wifi.post_sample(sample);
      }
    } else {
      // wakeup goal - clear display
      display.clear();
    }
  }

  // sleep
  sleep.plan_deep_sleep();
}

void loop() {
  // never happens
}