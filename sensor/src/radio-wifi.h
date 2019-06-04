#pragma once

#include <chrono>

#include "config.h"
#include "sample.h"

class RadioWiFi {
 public:
  bool connect(std::string ssid, std::string pass);
  bool connect();
  static void disconnect();

  void post_sample(const Sample &s);

 private:
  std::string ssid_;
  std::string pass_;

  std::chrono::milliseconds latest_connect_attempt_time;
};