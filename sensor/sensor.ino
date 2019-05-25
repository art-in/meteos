#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Preferences.h>
#include <WiFi.h>
#include <soc/rtc.h>
#include <sys/time.h>

#include "src/libs/Adafruit_BME280_Library/Adafruit_BME280.h"
#include "src/libs/MH-Z19-1.4.2/src/MHZ19.h"
#include "src/libs/U8g2_Arduino-2.25.10/src/U8g2lib.h"

#include "src/button.h"
#include "src/sample.h"

// constants

#define SEALEVELPRESSURE_HPA (1013.25)
#define PASCAL_TO_MECURY_MM (0.00750062)

#define MHZ_BAUDRATE 9600

#define SERVICE_UUID "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6"
#define SSID_CHARACTERISTIC_UUID "d1fe36b7-3223-4927-ada5-422f2b5afcf9"
#define PASS_CHARACTERISTIC_UUID "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9"

#define BUTTON_PIN_BITMASK 0x8000  // pin 15 (2^15 in hex)

#define uS_TO_S_FACTOR 1000000
#define uS_TO_MS_FACTOR 1000

const char *host = "192.168.1.100";
constexpr int port = 3300;

constexpr unsigned long CO2_WARMING_READ_PERIOD_MS = 30 * 1000L;

constexpr unsigned long SAMPLE_DELAY_US = 10000000;  // us
constexpr unsigned long DISPLAY_DELAY_US = 5000000;  // us

// rtc data

RTC_DATA_ATTR int wakeup_number = 0;
RTC_DATA_ATTR Sample last_sample;
RTC_DATA_ATTR uint64_t next_sample_time_us;
RTC_DATA_ATTR uint64_t clear_display_time_us;
RTC_DATA_ATTR uint64_t wakeup_time_us;
RTC_DATA_ATTR int wakeup_timer_mistake_us;
RTC_DATA_ATTR bool is_display_on = false;
RTC_DATA_ATTR bool is_wakeup_by_timer_to_sample;  // otherwise to clear display
RTC_DATA_ATTR int display_reading_idx = 0;
RTC_DATA_ATTR uint64_t before_sample_since_boot_duration_us = 0;
RTC_DATA_ATTR uint64_t before_clear_display_since_boot_duration_us = 0;

// current boot data

Preferences preferences;
HardwareSerial mhz_serial(2);

Adafruit_BME280 bme;  // I2C
MHZ19 mhz;            // UART

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);  // I2C

std::string ssid;
std::string pass;

// functions

Sample takeSample();
void printSample(const Sample &s);
void postSample(const Sample &s);

uint64_t get_epoch_time_us() {
  timeval time;
  timezone tz;
  gettimeofday(&time, &tz);
  return static_cast<uint64_t>(time.tv_sec) * uS_TO_S_FACTOR +
         static_cast<uint64_t>(time.tv_usec);
}

void log(String str, bool to_display = false) {
  Serial.print(str);

  // if (to_display) {
  //   display.clearBuffer();
  //   display.setFontPosTop();

  //   display.setFont(u8g2_font_5x7_mf);
  //   // randomize y-position to avoid OLED pixels burnout.
  //   display.drawStr(0, random(59), str.c_str());
  //   display.sendBuffer();
  // }
}

void log(uint64_t num) {
  uint64_t xx = num / 1000000000ULL;
  if (xx > 0) Serial.print((long)xx);
  Serial.print((long)(num - xx * 1000000000));
}

void log_ln(String str, bool to_display = false) {
  log(str + "\n", to_display);
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { log_ln("BT: client connected.", true); };

  void onDisconnect(BLEServer *pServer) {
    log_ln("BT: client disconnected.", true);
  }
};

class SSIDCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    log("BT: SSID reading... ");
    log_ln(pCharacteristic->getValue().c_str());
    log_ln("BT: SSID read.");
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    log("BT: SSID written: ");
    ssid = pCharacteristic->getValue();
    preferences.putString("ssid", ssid.c_str());
    log_ln(ssid.c_str());
  }
};

class PassCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    log("Pass read: ");
    log_ln(pCharacteristic->getValue().c_str());
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    log("Pass written: ");
    pass = pCharacteristic->getValue();
    preferences.putString("pass", pass.c_str());
    log_ln(pass.c_str());
  }
};

void read_preferences() {
  log_ln("reading prefs...", true);
  auto before_ms = millis();

  preferences.begin("meteos", false);

  ssid = preferences.getString("ssid", "").c_str();
  pass = preferences.getString("pass", "").c_str();

  log("SSID: ");
  log_ln(ssid.c_str());
  log("Pass: ");
  log_ln(pass.c_str());
  log_ln("reading prefs...done in " + String(millis() - before_ms) + "ms",
         true);
}

void init_display() {
  log_ln("init display...", true);
  auto before_ms = millis();

  display.begin();
  display.setContrast(255);

  log_ln("init display...done in " + String(millis() - before_ms) + "ms", true);
}

void init_bluetooth() {
  log_ln("init bluetooth...", true);
  auto before_ms = millis();

  BLEDevice::init("Meteos Sensor");
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *ssidCharacteristic = pService->createCharacteristic(
      SSID_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *passCharacteristic = pService->createCharacteristic(
      PASS_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  ssidCharacteristic->setValue(ssid);
  passCharacteristic->setValue(pass);

  ssidCharacteristic->setCallbacks(new SSIDCharacteristicCallbacks());
  passCharacteristic->setCallbacks(new PassCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(
      0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  log_ln("init bluetooth...done in " + String(millis() - before_ms) + "ms",
         true);
}

void wait_wifi_config() {
  if (ssid.empty() || pass.empty()) {
    log_ln("waiting wifi config...", true);
    init_bluetooth();
    while (ssid.empty() || pass.empty()) {
      delay(1000);
    }
    log_ln("waiting wifi config...done", true);
  }
}

void init_wifi() {
  log_ln("init wifi...", true);
  auto before_ms = millis();

  log("connecting to ");
  log(ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    log(".");
  }

  log_ln("");
  log_ln("IP address: ");
  log_ln(WiFi.localIP().toString());
  log_ln("init wifi...done in " + String(millis() - before_ms) + "ms", true);
}

void init_bme(bool is_initial_boot) {
  log_ln("init bme280...", true);
  auto before_ms = millis();

  if (!bme.begin(&Wire)) {
    log_ln("Could not find BME280 sensor.");
    while (1)
      ;
  }

  if (is_initial_boot) {
    // weather monitoring
    // suggested rate is 1/60Hz (1m)
    log_ln("bme: set sampling config", true);
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,  // temperature
                    Adafruit_BME280::SAMPLING_X1,  // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_OFF);
  }

  log_ln("init bme280...done in " + String(millis() - before_ms) + "ms", true);
}

void init_mhz(bool is_initial_boot) {
  log_ln("init mhz19...", true);
  auto before_ms = millis();

  mhz_serial.begin(MHZ_BAUDRATE);

  // mhz.printCommunication(false, true);
  mhz.begin(mhz_serial);
  mhz.setFilter();

  if (is_initial_boot) {
    mhz.setRange(2000);
    mhz.autoCalibration(true);

    log_ln("mhz: warming up...", true);
    while (true) {
      int co2 = mhz.getCO2(false, true);

      if (mhz.errorCode == RESULT_FILTER) {
        log_ln("mhz: warming up...", true);
      } else if (mhz.errorCode != RESULT_OK) {
        log_ln("mhz: failed to read co2 on warmup.", true);
      } else {
        log_ln("mhz: warmed up!", true);
        break;
      }

      delay(CO2_WARMING_READ_PERIOD_MS);
    }
    log_ln("mhz: warming up...done", true);
  }

  log_ln("init mhz19...done in " + String(millis() - before_ms) + "ms", true);
}

Sample takeSample() {
  log_ln("taking sample...", true);
  auto before_ms = millis();

  Sample s;

  bme.takeForcedMeasurement();

  s.temperature = bme.readTemperature();
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MECURY_MM;

  s.co2 = mhz.getCO2(false, true);

  if (mhz.errorCode != RESULT_OK) {
    log_ln("mhz: failed to receive co2. error: " + mhz.errorCode, true);
  }

  log_ln("taking sample...done in " + String(millis() - before_ms) + "ms",
         true);
  return s;
}

void printSample(const Sample &s) {
  log_ln("Temperature = " + String(s.temperature) + " *C");
  log_ln("Humidity = " + String(s.humidity) + " %");
  log_ln("Pressure = " + String(s.pressure) + " mm hg");
  log_ln("CO2 = " + String(s.co2) + " ppx");
}

void postSample(const Sample &s) {
  log_ln("posting sample...");
  auto before_ms = millis();

  WiFiClient client;

  if (!client.connect(host, port)) {
    log_ln("wifi error: client connection failed", true);
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
      log_ln("wifi error: client timeout", true);
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

  log_ln("posting sample...done in " + String(millis() - before_ms) + "ms",
         true);
}

void on_button_press() {
  if (is_display_on) {
    display_reading_idx++;

    if (display_reading_idx > 4) {
      display_reading_idx = 0;
    }
  } else {
    display_reading_idx = 0;
  }

  is_display_on = true;
}

void display_sample(Sample sample) {
  log_ln("displaying sample...");
  auto before_ms = millis();

  if (display_reading_idx == 4) {
    display_clear();
  } else {
    String title;
    String value;
    String value_postfix;

    switch (display_reading_idx) {
      case 0:
        title = "temperature";
        value = String(sample.temperature);
        value_postfix = "C";
        break;
      case 1:
        title = "humidity";
        value = String(sample.humidity);
        value_postfix = "%";
        break;
      case 2:
        title = "pressure";
        value = String(sample.pressure);
        value_postfix = "mm";
        break;
      case 3:
        title = "co2";
        value = String(sample.co2);
        value_postfix = "ppx";
        break;
    }

    display.clearBuffer();
    display.setFontPosTop();

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(100, 33, value_postfix.c_str());

    display.sendBuffer();

    auto now_us = get_epoch_time_us();
    clear_display_time_us = now_us + DISPLAY_DELAY_US;
  }

  log_ln("displaying sample...done in " + String(millis() - before_ms) + "ms");
}

void display_clear() {
  before_clear_display_since_boot_duration_us = millis() * uS_TO_MS_FACTOR;

  if (is_display_on) {
    is_display_on = false;

    display.clearBuffer();
    display.sendBuffer();

    log_ln("display cleared", true);
  }
}

void take_and_post_sample() {
  int mistake = get_epoch_time_us() - next_sample_time_us;
  log_ln("sample time mistake: " + String(mistake) + "us");

  before_sample_since_boot_duration_us = millis() * uS_TO_MS_FACTOR;
  next_sample_time_us = get_epoch_time_us() + SAMPLE_DELAY_US;

  last_sample = takeSample();
  // printSample(last_sample);
  postSample(last_sample);
}

void print_wakeup_reason(esp_sleep_wakeup_cause_t wakeup_reason) {
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      log_ln("wakeup cause: RTC_IO", true);
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      log_ln("wakeup cause: RTC_CNTL", true);
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      log_ln("wakeup cause: timer", true);
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      log_ln("wakeup cause: touchpad", true);
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      log_ln("wakeup cause: ULP program", true);
      break;
    default:
      log_ln("wakeup not after deep sleep", true);
  }
}

esp_sleep_wakeup_cause_t on_wakeup() {
  timeval time;
  timezone tz;
  gettimeofday(&time, &tz);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    wakeup_timer_mistake_us = get_epoch_time_us() - wakeup_time_us;
  }

  log_ln("--------");
  log_ln("wakeup number: " + String(wakeup_number));
  log("time: " + String(static_cast<unsigned int>(time.tv_sec)) + "s (");
  log(get_epoch_time_us());
  log_ln("us)");
  log_ln("wakeup time mistake: " + String(wakeup_timer_mistake_us) + "us");

  print_wakeup_reason(wakeup_reason);

  ++wakeup_number;

  return wakeup_reason;
}

void deep_sleep(int us) {
  log_ln("going to sleep for " + String(us / uS_TO_MS_FACTOR) + "ms...", true);

  // using ext1 with bitmask instead of ext0, since ext1 does not require RTC
  // peripherals to be powered on. by default, ESP32 will automatically power
  // down the peripherals not needed by the wakeup source.
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  // also wakeup by timer
  esp_sleep_enable_timer_wakeup(us);

  Serial.flush();

  // disconnect wifi otherwise may not connect on next boot.
  WiFi.disconnect(true);

  esp_deep_sleep_start();
}

void plan_deep_sleep() {
  log("before_sample_since_boot_duration_us: ");
  log(before_sample_since_boot_duration_us);
  log_ln("");

  auto sample_wakeup_time_us =
      next_sample_time_us - before_sample_since_boot_duration_us;
  auto clear_display_wakeup_time_us =
      clear_display_time_us - before_clear_display_since_boot_duration_us;

  log("sample_wakeup_time_us: ");
  log(sample_wakeup_time_us);
  log_ln("");

  log("clear_display_wakeup_time_us: ");
  log(clear_display_wakeup_time_us);
  log_ln("");

  // wakeup to take sample
  auto next_wakeup_time_us = sample_wakeup_time_us;
  is_wakeup_by_timer_to_sample = true;

  // or wakeup to clear display
  if (is_display_on && (clear_display_wakeup_time_us < next_wakeup_time_us)) {
    next_wakeup_time_us = clear_display_wakeup_time_us;
    is_wakeup_by_timer_to_sample = false;
  }

  if (is_wakeup_by_timer_to_sample) {
    log_ln("next timer wakeup goal: make sample");
  } else {
    log_ln("next timer wakeup goal: clear display");
  }

  auto now_us = get_epoch_time_us();

  next_wakeup_time_us -= wakeup_timer_mistake_us;
  wakeup_time_us = next_wakeup_time_us;

  if (now_us > next_wakeup_time_us) {
    log_ln("restarting instead of deep sleep...");
    ESP.restart();
  } else {
    int time_to_sleep_us = next_wakeup_time_us - now_us;
    deep_sleep(time_to_sleep_us);
  }
}

void setup() {
  Serial.begin(115200);
  auto before_ms = millis();

  // wake up
  auto wakeup_reason = on_wakeup();

  // always init display for debug log
  // init_display();

  bool is_wakeup_by_button = wakeup_reason == ESP_SLEEP_WAKEUP_EXT1;
  bool is_wakeup_by_timer = wakeup_reason == ESP_SLEEP_WAKEUP_TIMER;
  bool is_initial_boot = !is_wakeup_by_button && !is_wakeup_by_timer;

  if (is_wakeup_by_button) {
    // wakeup by display button

    on_button_press();

    // display
    init_display();
    display_sample(last_sample);

  } else {
    // wakeup by timer or initial boot

    // init
    if (is_initial_boot || is_wakeup_by_timer_to_sample) {
      read_preferences();
      wait_wifi_config();
      init_wifi();
      init_bme(is_initial_boot);
      init_mhz(is_initial_boot);

      // sample
      take_and_post_sample();
    } else {
      init_display();
      display_clear();
    }
  }

  // sleep
  log_ln("this boot duration: " + String(millis() - before_ms) + "ms");
  plan_deep_sleep();

  log_ln("setup tail - should never happen!", true);
}

void loop() { log_ln("loop - should never happen!", true); }