#include <Arduino.h>
#include <WiFi.h>

#include "radio-wifi.h"
#include "utils.h"

// TODO: move to config
const char* host = "192.168.1.100";
constexpr int port = 3300;

constexpr int CONNECT_TIMEOUT = 5000;

bool RadioWiFi::connect() {
  log_ln("wifi: connect...", true);
  auto before_ms = millis();

  log("wifi: connecting to ");
  log(ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    log(".");

    if (millis() - before_ms > CONNECT_TIMEOUT) {
      log_ln("");
      log_ln("wifi: error: connect timeout");
      return false;
    }
  }

  log_ln("");
  log_ln("wifi: IP address: " + WiFi.localIP().toString());
  log_ln("wifi: connect...done in " + String(millis() - before_ms) + "ms",
         true);
  return true;
}

void RadioWiFi::post_sample(const Sample& s) {
  log_ln("wifi: posting sample...");
  auto before_ms = millis();

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

  // log_ln("---");
  // log(msg);
  // log_ln("---");

  client.print(msg);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      log_ln("wifi: error: client timeout", true);
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  // NOTE: Blocks button state read.
  // while (client.available()) {
  //   String line = client.readStringUntil('\r');
  //   log(line);
  // }

  client.stop();

  log_ln(
      "wifi: posting sample...done in " + String(millis() - before_ms) + "ms",
      true);
}

void RadioWiFi::disconnect() {
  WiFi.disconnect(true);
  log_ln("wifi: disconnected");
}