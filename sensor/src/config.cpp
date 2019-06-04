#include "config.h"
#include "utils.h"

constexpr int WIFI_CONFIG_WAIT_DELAY = 3000;

void Config::init() {
  METEOS_SCOPED_LOGGER("config: init");

  preferences.begin("meteos");

  wifi_ssid_ = preferences.getString("ssid", "").c_str();
  wifi_pass_ = preferences.getString("pass", "").c_str();

  METEOS_LOG("config: ssid: ");
  METEOS_LOG_LN(wifi_ssid_.c_str());
  METEOS_LOG("config: pass: ");
  METEOS_LOG_LN(wifi_pass_.c_str());
}

std::string Config::wifi_ssid() const { return wifi_ssid_; }

void Config::wifi_ssid(std::string ssid) {
  METEOS_LOG_LN(String("config: set ssid: ") + ssid.c_str());
  wifi_ssid_ = ssid;
  preferences.putString("ssid", ssid.c_str());
}

std::string Config::wifi_pass() const { return wifi_pass_; }

void Config::wifi_pass(std::string pass) {
  METEOS_LOG_LN(String("config: set pass: ") + pass.c_str());
  wifi_pass_ = pass;
  preferences.putString("pass", pass.c_str());
}

bool Config::has_wifi_config() {
  return !wifi_ssid_.empty() && !wifi_pass_.empty();
}

void Config::wait_wifi_config(std::function<void()> step) {
  METEOS_SCOPED_LOGGER("config: wait wifi config");
  while (!has_wifi_config()) {
    if (step) {
      step();
    }
    delay(WIFI_CONFIG_WAIT_DELAY);
  }
}

void Config::clear() {
  METEOS_LOG_LN("config: clear");
  preferences.clear();
  wifi_ssid_ = "";
  wifi_pass_ = "";
}