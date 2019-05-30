#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <functional>

class Config {
 public:
  void init();

  std::string wifi_ssid() const;
  void wifi_ssid(std::string);

  std::string wifi_pass() const;
  void wifi_pass(std::string);

  bool has_wifi_config();
  void wait_wifi_config(std::function<void()> step);

  void clear();

 private:
  Preferences preferences;

  std::string wifi_ssid_;
  std::string wifi_pass_;
};