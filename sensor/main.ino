#include <Arduino.h>
#include <chrono>
#include <functional>

#include "src/button.h"
#include "src/config.h"
#include "src/display.h"
#include "src/radio-bt.h"
#include "src/radio-wifi.h"
#include "src/sample.h"
#include "src/sensors.h"
#include "src/utils.h"

constexpr auto SAMPLE_DELAY_US = std::chrono::milliseconds{10000};
constexpr auto DISPLAY_DELAY_US = std::chrono::milliseconds{5000};
constexpr int PIN_BUTTON = 15;

Display display;
Config config;
Sensors sensors;
Button btn;
RadioBT bt{config};
RadioWiFi wifi;

std::chrono::milliseconds latest_sample_time;
std::chrono::milliseconds latest_display_reading_time;

void setup() {
  Serial.begin(115200);
  log_ln("main: setup.");

  display.init();
  display.draw_logo();

  config.init();
  sensors.init();
  bt.init();
  btn.init(PIN_BUTTON, on_button_state_changed);

  // use initial boot as only time to update wifi config
  // TODO: add config mode (long button press)
  // config.clear();

  if (!config.has_wifi_config()) {
    bt.start_advertising();

    config.wait_wifi_config([]() { display.draw_wait_config(); });
    display.clear();

    bt.stop_advertising();
  }

  wifi.connect(config.wifi_ssid(), config.wifi_pass());

  // delay logo
  delay(1000);
  display.clear();
}

void loop() {
  auto now = time();

  loop_sample(now);
  loop_display(now);
  btn.update();

  delay(10);
}

void loop_sample(const std::chrono::milliseconds now) {
  if (now - latest_sample_time < SAMPLE_DELAY_US) {
    return;
  }

  latest_sample_time = now;

  auto sample = sensors.take_sample();
  wifi.post_sample(sample);
}

void loop_display(const std::chrono::milliseconds now) {
  if (!display.is_on() ||
      (now - latest_display_reading_time < DISPLAY_DELAY_US)) {
    return;
  }

  display.clear();
}

void on_button_state_changed(ButtonState state) {
  if (state == ButtonState::PRESSED) {
    latest_display_reading_time = time();

    auto latest_sample = sensors.get_latest_sample();
    display.draw_next_reading(latest_sample);
  }
}