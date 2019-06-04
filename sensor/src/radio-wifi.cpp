#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "radio-wifi.h"
#include "utils.h"

constexpr auto CONNECT_TIMEOUT = std::chrono::seconds{4};
constexpr auto RECONNECT_DELAY = std::chrono::minutes{5};

void RadioWiFi::init(std::string ssid, std::string pass, std::string host,
                     int port) {
  ssid_ = ssid;
  pass_ = pass;
  host_ = host;
  port_ = port;
}

bool RadioWiFi::connect(bool ignore_reconnect_delay) {
  METEOS_SCOPED_LOGGER("wifi: connect");

  if (ssid_.empty() || pass_.empty() || host_.empty() || port_ == 0) {
    METEOS_LOG_LN("wifi: skipping connect per insufficient config");
    return false;
  }

  auto now = time();

  if (!ignore_reconnect_delay && (latest_connect_attempt_time.count()) &&
      (now - latest_connect_attempt_time < RECONNECT_DELAY)) {
    // do not try to connect too often in case AP is unavailable to save power.
    METEOS_LOG_LN("wifi: skipping connect per reconnect delay");
    return false;
  }

  latest_connect_attempt_time = now;

  METEOS_LOG("wifi: connecting to ");
  METEOS_LOG(ssid_.c_str());

  WiFi.begin(ssid_.c_str(), pass_.c_str());

  auto start = time();
  wl_status_t status;
  while ((status = WiFi.status()) != WL_CONNECTED) {
    if (status == wl_status_t::WL_NO_SSID_AVAIL) {
      METEOS_LOG_LN("");
      METEOS_LOG_LN("wifi: error: AP with specified SSID is not available");
      return false;
    }

    if (status == wl_status_t::WL_CONNECT_FAILED) {
      METEOS_LOG_LN("");
      METEOS_LOG_LN("wifi: error: connect failed");
      return false;
    }

    if (time() - start > CONNECT_TIMEOUT) {
      METEOS_LOG_LN("");
      METEOS_LOG_LN("wifi: error: connect timeout");
      return false;
    }

    METEOS_LOG(".");
    delay(100);
  }
  METEOS_LOG_LN("");
  METEOS_LOG_LN("wifi: local IP: " + WiFi.localIP().toString());

  int res;
  if ((res = esp_wifi_set_ps(WIFI_PS_MAX_MODEM)) == ESP_OK) {
    METEOS_LOG_LN("wifi: set wifi power save mode");
  } else {
    METEOS_LOG("wifi: error: failed to set wifi power save mode: ");
    METEOS_LOG_LN(String(res));
  }

  return true;
}

void RadioWiFi::post_sample(const Sample& s) {
  METEOS_SCOPED_LOGGER("wifi: post sample");

  if (WiFi.status() != WL_CONNECTED && !connect()) {
    return;
  }

  WiFiClient client;

  if (!client.connect(host_.c_str(), port_)) {
    METEOS_LOG_LN("wifi: error: client connection failed");
    return;
  }

  auto json = String(R"({"t":)") + s.temperature +
              R"(,"h":)" + s.humidity + R"(,"p":)" + s.pressure + R"(,"c":)" +
              s.co2 + R"(})";

  auto msg = String("POST /samples HTTP/1.1\r\n") + "Host: " + host_.c_str() +
             ":" + port_ + "\r\n" + "Content-Type: application/json\r\n" +
             "Content-Length: " + json.length() + "\r\n" + "\r\n" + json;

  client.print(msg);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      METEOS_LOG_LN("wifi: error: client timeout");
      client.stop();
      return;
    }
  }

  client.stop();
}

void RadioWiFi::disconnect() {
  WiFi.disconnect(true);
  METEOS_LOG_LN("wifi: disconnected");
}