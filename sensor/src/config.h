#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <functional>

class Config {
 public:
  Config();
  void init();

  std::string wifi_ssid;
  std::string wifi_pass;

  void set_wifi_ssid(std::string);
  void set_wifi_pass(std::string);

  bool has_wifi_config();
  void wait_wifi_config(std::function<void()> step);

  void clear();

 private:
  Preferences preferences;
};