#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "radio-wifi.h"
#include "utils.h"

constexpr auto CONNECT_TIMEOUT = std::chrono::seconds{4};
constexpr auto RECONNECT_DELAY = std::chrono::minutes{5};

// TODO: move to config
const char* host = "192.168.1.100";
constexpr int port = 3300;

bool RadioWiFi::connect() { return connect(ssid_, pass_); }

bool RadioWiFi::connect(std::string ssid, std::string pass) {
  log_ln("wifi: connect...", true);
  auto before_ms = millis();

  ssid_ = ssid;
  pass_ = pass;

  auto now = time();

  if ((latest_connect_attempt_time.count()) &&
      (now - latest_connect_attempt_time < RECONNECT_DELAY)) {
    // in case AP is unavailable do not try to connect too often to save power.
    log_ln("wifi: skipping connect per delay");
    return false;
  }

  latest_connect_attempt_time = now;

  log("wifi: connecting to ");
  log(ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  auto start = time();
  wl_status_t status;
  while ((status = WiFi.status()) != WL_CONNECTED) {
    if (status == wl_status_t::WL_NO_SSID_AVAIL) {
      log_ln("");
      log_ln("wifi: error: AP with specified SSID is not available");
      return false;
    }

    if (status == wl_status_t::WL_CONNECT_FAILED) {
      log_ln("");
      log_ln("wifi: error: connect failed");
      return false;
    }

    if (time() - start > CONNECT_TIMEOUT) {
      log_ln("");
      log_ln("wifi: error: connect timeout");
      return false;
    }

    log(".");
    delay(100);
  }
  log_ln("");
  log_ln("wifi: local IP: " + WiFi.localIP().toString());

  int res;
  if ((res = esp_wifi_set_ps(WIFI_PS_MAX_MODEM)) == ESP_OK) {
    log_ln("wifi: set wifi power save mode");
  } else {
    log("wifi: error: failed to set wifi power save mode: ");
    log_ln(String(res));
  }

  log_ln("wifi: connect...done in " + String(millis() - before_ms) + "ms",
         true);
  return true;
}

void RadioWiFi::post_sample(const Sample& s) {
  log_ln("wifi: posting sample...");
  auto before_ms = millis();

  if (WiFi.status() != WL_CONNECTED && !connect()) {
    return;
  }

  WiFiClient client;

  if (!client.connect(host, port)) {
    log_ln("wifi: error: client connection failed", true);
    return;
  }

  auto json = String(R"({"t":)") + s.temperature +
              R"(,"h":)" + s.humidity + R"(,"p":)" + s.pressure + R"(,"c":)" +
              s.co2 + R"(})";

  auto msg = String("POST /samples HTTP/1.1\r\n") + "Host: " + host + ":" +
             port + "\r\n" + "Content-Type: application/json\r\n" +
             "Content-Length: " + json.length() + "\r\n" + "\r\n" + json;

  client.print(msg);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      log_ln("wifi: error: client timeout", true);
      client.stop();
      return;
    }
  }

  client.stop();

  log_ln(
      "wifi: posting sample...done in " + String(millis() - before_ms) + "ms",
      true);
}

void RadioWiFi::disconnect() {
  WiFi.disconnect(true);
  log_ln("wifi: disconnected");
}