#include <Arduino.h>
#include <chrono>

#include "src/button.h"
#include "src/config.h"
#include "src/display.h"
#include "src/radio-bt.h"
#include "src/radio-wifi.h"
#include "src/sample.h"
#include "src/sensors.h"
#include "src/utils.h"

constexpr auto DISPLAY_DELAY_US = std::chrono::seconds{5};
constexpr auto BUTTON_CONFIG_MODE_DELAY = std::chrono::seconds{1};
constexpr int PIN_BUTTON = 15;

Display display;
Config config;
Sensors sensors;
Button btn;
RadioBT bt{config};
RadioWiFi wifi;

std::chrono::milliseconds latest_sample_time;
std::chrono::milliseconds latest_display_reading_time;
std::chrono::milliseconds latest_display_config_time;

bool in_config_mode = false;

void setup() {
  Serial.begin(115200);
  METEOS_LOG_LN("main: setup.");

  display.init();
  display.draw_logo();

  config.init();
  sensors.init();
  bt.init();
  btn.init(PIN_BUTTON, on_button_state_changed);
  wifi.init(config.wifi_ssid(), config.wifi_pass(), config.backend_host(),
            config.backend_port());
  wifi.connect();

  display.clear();
}

void loop() {
  auto now = time();

  if (!in_config_mode) {
    loop_sample(now);
  }

  loop_display(now);
  btn.update();

  delay(10);
}

void loop_sample(const std::chrono::milliseconds now) {
  if (now - latest_sample_time < config.sample_delay()) {
    return;
  }

  latest_sample_time = now;

  auto sample = sensors.take_sample();
  wifi.post_sample(sample);
}

void loop_display(const std::chrono::milliseconds now) {
  if (display.is_reading_shown() &&
      (now - latest_display_reading_time >= DISPLAY_DELAY_US)) {
    display.clear();
  }

  if (in_config_mode &&
      (now - latest_display_config_time >= DISPLAY_DELAY_US)) {
    latest_display_config_time = now;
    display.draw_wait_config();
  }
}

void on_button_state_changed(ButtonState new_state,
                             std::chrono::microseconds prev_state_duration) {
  if (new_state == ButtonState::RELEASED) {
    if (in_config_mode) {
      exit_config_mode();
    } else if (prev_state_duration >= BUTTON_CONFIG_MODE_DELAY) {
      enter_config_mode();
    } else {
      latest_display_reading_time = time();

      auto latest_sample = sensors.get_latest_sample();
      display.draw_next_reading(latest_sample);
    }
  }
}

void enter_config_mode() {
  METEOS_SCOPED_LOGGER("main: enter config mode");

  in_config_mode = true;
  bt.start_advertising();
}

void exit_config_mode() {
  METEOS_SCOPED_LOGGER("main: exit config mode");

  in_config_mode = false;
  bt.stop_advertising();
  display.clear();

  wifi.init(config.wifi_ssid(), config.wifi_pass(), config.backend_host(),
            config.backend_port());
  wifi.connect(true);
}
