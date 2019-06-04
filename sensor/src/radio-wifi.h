#pragma once

#include <chrono>

#include "config.h"
#include "sample.h"

class RadioWiFi {
 public:
  void init(std::string ssid, std::string pass, std::string host, int port);

  bool connect(bool ignore_reconnect_delay = false);
  static void disconnect();

  void post_sample(const Sample &s);

 private:
  std::string ssid_;
  std::string pass_;
  std::string host_;
  int port_;

  std::chrono::milliseconds latest_connect_attempt_time;
};