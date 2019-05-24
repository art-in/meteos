#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Preferences.h>
#include <WiFi.h>

#include "src/button.h"
#include "src/libs/Adafruit_BME280_Library/Adafruit_BME280.h"
#include "src/libs/MH-Z19-1.4.2/src/MHZ19.h"
#include "src/libs/U8g2_Arduino-2.25.10/src/U8g2lib.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define PASCAL_TO_MECURY_MM (0.00750062)

#define MHZ_BAUDRATE 9600
constexpr unsigned long CO2_READ_PERIOD_MS = 10 * 1000L;
constexpr unsigned long CO2_WARMING_READ_PERIOD_MS = 30 * 1000L;

HardwareSerial mhz_serial(2);

Adafruit_BME280 bme;  // I2C
MHZ19 mhz;            // UART

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);  // I2C

const char *host = "192.168.1.100";
const int port = 3300;

constexpr unsigned long SAMPLE_DELAY = 10000;
constexpr unsigned long DISPLAY_DELAY = 5000;

constexpr int PIN_BUTTON_DISPLAY = 15;

#define SERVICE_UUID "b8acbbea-21c2-4c6a-a52f-6033f2f6d1a6"
#define SSID_CHARACTERISTIC_UUID "d1fe36b7-3223-4927-ada5-422f2b5afcf9"
#define PASS_CHARACTERISTIC_UUID "5e527e75-2d5a-40f1-95fb-0e4a7d612ee9"

std::string ssid;
std::string pass;

Preferences preferences;

struct Sample {
  float temperature;
  float humidity;
  float pressure;
  int co2;
};

Sample takeSample();
void printSample(const Sample &s);
void postSample(const Sample &s);

unsigned long last_sample_time;
unsigned long last_display_update;
Sample last_sample;

bool is_display_on = false;
bool is_display_updated = true;
unsigned long display_on_time;
int display_reading_idx = 0;

Button display_btn{PIN_BUTTON_DISPLAY};

void log(String str, bool to_display = false) {
  Serial.print(str);

  if (to_display) {
    display.clearBuffer();
    display.setFontPosTop();

    display.setFont(u8g2_font_5x7_mf);
    // randomize y-position to avoid OLED pixels burnout.
    display.drawStr(0, random(59), str.c_str());
    display.sendBuffer();
  }
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

void setup() {
  Serial.begin(115200);

  log_ln("Reading preferences...", true);
  preferences.begin("meteos", false);

  ssid = preferences.getString("ssid", "").c_str();
  pass = preferences.getString("pass", "").c_str();

  log("SSID: ");
  log_ln(ssid.c_str());
  log("Pass: ");
  log_ln(pass.c_str());
  log_ln("Reading preferences...done", true);

  // init button
  log_ln("Init button...", true);
  display_btn.on_state_changed = [](ButtonState btn_state) {
    if (btn_state == ButtonState::ON) {
      if (is_display_on) {
        display_reading_idx++;

        if (display_reading_idx > 4) {
          display_reading_idx = 0;
        }
      } else {
        display_reading_idx = 0;
      }

      is_display_on = true;
      is_display_updated = false;
      display_on_time = millis();
    }
  };
  log_ln("Init button...done", true);

  // init display
  log_ln("init display...", true);
  display.begin();
  display.setContrast(255);
  log_ln("init display...done", true);

  // init bluetooth
  log_ln("init bluetooth...", true);
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

  log_ln("Waiting for ssid/pass configuration...", true);
  while (ssid.empty() || pass.empty()) {
    delay(1000);
  }
  log_ln("Waiting for ssid/pass configuration...done", true);
  log_ln("init bluetooth...done", true);

  // init wifi
  log_ln("init wifi...", true);
  log("Connecting to ");
  log_ln(ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    log(".");
  }

  log_ln("");
  log_ln("WiFi connected");
  log_ln("IP address: ");
  log_ln(WiFi.localIP().toString());
  log_ln("init wifi...done", true);

  // init BME280
  log_ln("init bme280...", true);
  if (!bme.begin(&Wire)) {
    log_ln("Could not find BME280 sensor.");
    while (1)
      ;
  }

  // weather monitoring
  // suggested rate is 1/60Hz (1m)
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,  // temperature
                  Adafruit_BME280::SAMPLING_X1,  // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_OFF);
  log_ln("init bme280...done", true);

  // init MHZ19
  log_ln("init mhz19...", true);
  mhz_serial.begin(MHZ_BAUDRATE);

  // mhz.printCommunication(false, true);
  mhz.begin(mhz_serial);
  mhz.setFilter();

  mhz.setRange(2000);
  mhz.autoCalibration(true);

  while (true) {
    int co2 = mhz.getCO2(false, true);

    if (mhz.errorCode == RESULT_FILTER) {
      log_ln("mhz: ... warming up ...", true);
    } else if (mhz.errorCode != RESULT_OK) {
      log_ln("mhz: failed to read co2 on warmup.", true);
    } else {
      log_ln("mhz: warmed up!", true);
      break;
    }

    delay(CO2_WARMING_READ_PERIOD_MS);
  }
  log_ln("init mhz19...done", true);
}

unsigned int posted_samples_count;

void loop() {
  auto now = millis();
  if (!last_sample_time || (now - last_sample_time >= SAMPLE_DELAY)) {
    log_ln("posting sample...", true);
    last_sample_time = now;

    last_sample = takeSample();

    // printSample(last_sample);
    postSample(last_sample);
    posted_samples_count++;
    log_ln("posting sample...done " + String(posted_samples_count), true);
  }

  if (is_display_on) {
    if (!is_display_updated) {
      is_display_updated = true;

      display_sample(last_sample);
    }

    if (now - display_on_time >= DISPLAY_DELAY) {
      display_off();
    }
  }

  display_btn.update();

  delay(10);
}

Sample takeSample() {
  Sample s;

  bme.takeForcedMeasurement();

  s.temperature = bme.readTemperature();
  s.humidity = bme.readHumidity();
  s.pressure = bme.readPressure() * PASCAL_TO_MECURY_MM;

  s.co2 = mhz.getCO2(false, true);

  if (mhz.errorCode != RESULT_OK) {
    log_ln("mhz: failed to receive co2. error: " + mhz.errorCode, true);
  }

  return s;
}

void printSample(const Sample &s) {
  log_ln("---");

  log("Temperature = ");
  log(s.temperature);
  log_ln(" *C");

  log("Humidity = ");
  log(s.humidity);
  log_ln(" %");

  log("Pressure = ");
  log(s.pressure);
  log_ln(" mm hg");

  log("CO2 = ");
  log(s.co2);
  log_ln(" ppx");
}

void postSample(const Sample &s) {
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
}

void display_sample(Sample sample) {
  display.clearBuffer();
  display.setFontPosTop();

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
    case 4:
      display_off();
  }

  if (display_reading_idx != 4) {
    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(0, 0, title.c_str());

    display.setFont(u8g2_font_inb19_mf);
    display.drawStr(0, 25, value.c_str());

    display.setFont(u8g2_font_t0_11_mf);
    display.drawStr(100, 33, value_postfix.c_str());

    display.sendBuffer();
  }
}

void display_off() {
  display.clearBuffer();
  display.sendBuffer();

  is_display_on = false;
}