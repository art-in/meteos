#include "config.h"
#include "utils.h"

constexpr int WIFI_CONFIG_WAIT_DELAY = 3000;

constexpr auto PREF_KEY_WIFI_SSID = "wifi_ssid";
constexpr auto PREF_KEY_WIFI_PASS = "wifi_pass";
constexpr auto PREF_KEY_BACKEND_HOST = "backend_host";
constexpr auto PREF_KEY_BACKEND_PORT = "backend_port";
constexpr auto PREF_KEY_SAMPLE_DELAY = "sample_delay";

void Config::init() {
  METEOS_SCOPED_LOGGER("config: init");

  preferences.begin("meteos");

  wifi_ssid_ = preferences.getString(PREF_KEY_WIFI_SSID, "").c_str();
  wifi_pass_ = preferences.getString(PREF_KEY_WIFI_PASS, "").c_str();
  backend_host_ = preferences.getString(PREF_KEY_BACKEND_HOST, "").c_str();
  backend_port_ = preferences.getInt(PREF_KEY_BACKEND_PORT, 0);
  sample_delay_ =
      std::chrono::seconds{preferences.getInt(PREF_KEY_SAMPLE_DELAY, 10)};

  METEOS_LOG_LN(String("config: wifi ssid: ") + wifi_ssid_.c_str());
  METEOS_LOG_LN(String("config: wifi pass: ") + wifi_pass_.c_str());
  METEOS_LOG_LN(String("config: backend host: ") + backend_host_.c_str());
  METEOS_LOG_LN(String("config: backend port: ") + backend_port_);
  METEOS_LOG_LN(String("config: sample delay sec: ") +
                (int)sample_delay_.count());
}

std::string Config::wifi_ssid() const { return wifi_ssid_; }

void Config::wifi_ssid(std::string ssid) {
  METEOS_LOG_LN(String("config: set ssid: ") + ssid.c_str());
  wifi_ssid_ = ssid;
  preferences.putString(PREF_KEY_WIFI_SSID, ssid.c_str());
}

std::string Config::wifi_pass() const { return wifi_pass_; }

void Config::wifi_pass(std::string pass) {
  METEOS_LOG_LN(String("config: set pass: ") + pass.c_str());
  wifi_pass_ = pass;
  preferences.putString(PREF_KEY_WIFI_PASS, pass.c_str());
}

std::string Config::backend_host() const { return backend_host_; }

void Config::backend_host(std::string host) {
  METEOS_LOG_LN(String("config: set host: ") + host.c_str());
  backend_host_ = host;
  preferences.putString(PREF_KEY_BACKEND_HOST, host.c_str());
}

int Config::backend_port() const { return backend_port_; }

void Config::backend_port(int port) {
  METEOS_LOG_LN(String("config: set port: ") + port);

  backend_port_ = port;
  preferences.putInt(PREF_KEY_BACKEND_PORT, port);
}

std::chrono::seconds Config::sample_delay() const { return sample_delay_; }

void Config::sample_delay(std::chrono::seconds sample_delay) {
  METEOS_LOG_LN(String("config: sample delay sec: ") +
                (int)sample_delay.count());

  sample_delay_ = sample_delay;
  preferences.putInt(PREF_KEY_SAMPLE_DELAY, sample_delay.count());
}

void Config::clear() {
  METEOS_LOG_LN("config: clear");

  preferences.clear();

  wifi_ssid_ = "";
  wifi_pass_ = "";
  backend_host_ = "";
  backend_port_ = 0;
  sample_delay_ = std::chrono::seconds{10};
}