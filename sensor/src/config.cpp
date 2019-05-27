#include "config.h"
#include "utils.h"

Config::Config() { preferences.begin("meteos", false); }

void Config::init() {
  log_ln("config: init...", true);
  auto before_ms = millis();

  wifi_ssid = preferences.getString("ssid", "").c_str();
  wifi_pass = preferences.getString("pass", "").c_str();

  log("config: ssid: ");
  log_ln(wifi_ssid.c_str());
  log("config: pass: ");
  log_ln(wifi_pass.c_str());
  log_ln("config: init...done in " + String(millis() - before_ms) + "ms", true);
}

void Config::set_wifi_ssid(std::string ssid) {
  log_ln(String("config: set ssid: ") + ssid.c_str());
  wifi_ssid = ssid;
  preferences.putString("ssid", ssid.c_str());
}

void Config::set_wifi_pass(std::string pass) {
  log_ln(String("config: set pass: ") + pass.c_str());
  wifi_pass = pass;
  preferences.putString("pass", pass.c_str());
}

bool Config::has_wifi_config() {
  return !wifi_ssid.empty() && !wifi_pass.empty();
}

void Config::wait_wifi_config(std::function<void()> step) {
  log_ln("config: waiting wifi config...", true);
  while (!has_wifi_config()) {
    if (step) {
      step();
    }
    delay(3000);
  }
  log_ln("config: waiting wifi config...done", true);
}

void Config::clear() {
  log_ln("config: clear");
  preferences.clear();
  wifi_ssid = "";
  wifi_pass = "";
}