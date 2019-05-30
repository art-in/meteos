#include "config.h"
#include "utils.h"

constexpr int WIFI_CONFIG_WAIT_DELAY = 3000;

void Config::init() {
  log_ln("config: init...", true);

  preferences.begin("meteos");

  auto before_ms = millis();

  wifi_ssid_ = preferences.getString("ssid", "").c_str();
  wifi_pass_ = preferences.getString("pass", "").c_str();

  log("config: ssid: ");
  log_ln(wifi_ssid_.c_str());
  log("config: pass: ");
  log_ln(wifi_pass_.c_str());
  log_ln("config: init...done in " + String(millis() - before_ms) + "ms", true);
}

std::string Config::wifi_ssid() const { return wifi_ssid_; }

void Config::wifi_ssid(std::string ssid) {
  log_ln(String("config: set ssid: ") + ssid.c_str());
  wifi_ssid_ = ssid;
  preferences.putString("ssid", ssid.c_str());
}

std::string Config::wifi_pass() const { return wifi_pass_; }

void Config::wifi_pass(std::string pass) {
  log_ln(String("config: set pass: ") + pass.c_str());
  wifi_pass_ = pass;
  preferences.putString("pass", pass.c_str());
}

bool Config::has_wifi_config() {
  return !wifi_ssid_.empty() && !wifi_pass_.empty();
}

void Config::wait_wifi_config(std::function<void()> step) {
  log_ln("config: waiting wifi config...", true);
  while (!has_wifi_config()) {
    if (step) {
      step();
    }
    delay(WIFI_CONFIG_WAIT_DELAY);
  }
  log_ln("config: waiting wifi config...done", true);
}

void Config::clear() {
  log_ln("config: clear");
  preferences.clear();
  wifi_ssid_ = "";
  wifi_pass_ = "";
}