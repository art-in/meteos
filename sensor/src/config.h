#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <chrono>
#include <functional>

class Config {
 public:
  void init();

  std::string wifi_ssid() const;
  void wifi_ssid(std::string);

  std::string wifi_pass() const;
  void wifi_pass(std::string);

  std::string backend_host() const;
  void backend_host(std::string);

  int backend_port() const;
  void backend_port(int);

  std::chrono::seconds sample_delay() const;
  void sample_delay(std::chrono::seconds);

  void clear();

 private:
  Preferences preferences;

  std::string wifi_ssid_;
  std::string wifi_pass_;
  std::string backend_host_;
  int backend_port_;
  std::chrono::seconds sample_delay_;
};