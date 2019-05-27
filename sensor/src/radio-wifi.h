#pragma once

#include "config.h"
#include "sample.h"

class RadioWiFi {
 public:
  RadioWiFi(std::string _ssid, std::string _pass) : ssid{_ssid}, pass{_pass} {};

  bool connect();
  void post_sample(const Sample &s);
  static void disconnect();

 private:
  std::string ssid;
  std::string pass;
};