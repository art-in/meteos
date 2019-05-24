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

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { Serial.println("Client connected."); };

  void onDisconnect(BLEServer *pServer) {
    Serial.println("Client disconnected.");
  }
};

class SSIDCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.print("BT: SSID reading... ");
    Serial.println(pCharacteristic->getValue().c_str());
    Serial.println("BT: SSID read.");
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.print("BT: SSID written: ");
    ssid = pCharacteristic->getValue();
    preferences.putString("ssid", ssid.c_str());
    Serial.println(ssid.c_str());
  }
};

class PassCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    Serial.print("Pass read: ");
    Serial.println(pCharacteristic->getValue().c_str());
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.print("Pass written: ");
    pass = pCharacteristic->getValue();
    preferences.putString("pass", pass.c_str());
    Serial.println(pass.c_str());
  }
};

void setup() {
  Serial.begin(115200);

  preferences.begin("meteos", false);

  ssid = preferences.getString("ssid", "").c_str();
  pass = preferences.getString("pass", "").c_str();

  Serial.print("SSID: ");
  Serial.println(ssid.c_str());
  Serial.print("Pass: ");
  Serial.println(pass.c_str());

  // init button
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

  // init display
  display.begin();
  display.setContrast(255);

  // init bluetooth
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

  Serial.println("Waiting for ssid/pass configuration.");
  while (ssid.empty() || pass.empty()) {
    delay(1000);
  }
  Serial.println("SSID/pass configured");

  // init wifi
  Serial.print("Connecting to ");
  Serial.println(ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // init BME280
  if (!bme.begin(&Wire)) {
    Serial.println("Could not find BME280 sensor.");
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

  // init MHZ19
  mhz_serial.begin(MHZ_BAUDRATE);

  // mhz.printCommunication(false, true);
  mhz.begin(mhz_serial);
  mhz.setFilter();

  mhz.setRange(2000);
  mhz.autoCalibration(true);

  while (true) {
    int co2 = mhz.getCO2(false, true);

    if (mhz.errorCode == RESULT_FILTER) {
      Serial.println("mhz: ... warming up ...");
    } else if (mhz.errorCode != RESULT_OK) {
      Serial.println("mhz: failed to read co2 on warmup.");
    } else {
      Serial.println("mhz: warmed up!");
      break;
    }

    delay(CO2_WARMING_READ_PERIOD_MS);
  }
}

void loop() {
  auto now = millis();
  if (!last_sample_time || (now - last_sample_time >= SAMPLE_DELAY)) {
    last_sample_time = now;

    last_sample = takeSample();

    // printSample(last_sample);
    postSample(last_sample);
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
    Serial.println("mhz: failed to receive co2. error: " + mhz.errorCode);
  }

  return s;
}

void printSample(const Sample &s) {
  Serial.println("---");

  Serial.print("Temperature = ");
  Serial.print(s.temperature);
  Serial.println(" *C");

  Serial.print("Humidity = ");
  Serial.print(s.humidity);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(s.pressure);
  Serial.println(" mm hg");

  Serial.print("CO2 = ");
  Serial.print(s.co2);
  Serial.println(" ppx");
}

void postSample(const Sample &s) {
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println("wifi error: client connection failed");
    return;
  }

  auto json = String(R"({"t":)") + s.temperature +
              R"(,"h":)" + s.humidity + R"(,"p":)" + s.pressure + R"(,"c":)" +
              s.co2 + R"(})";

  auto msg = String("POST /samples HTTP/1.1\r\n") + "Host: " + host + ":" +
             port + "\r\n" + "Content-Type: application/json\r\n" +
             "Content-Length: " + json.length() + "\r\n" + "\r\n" + json;

  // Serial.println("---");
  // Serial.print(msg);
  // Serial.println("---");

  client.print(msg);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println("wifi error: client timeout");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  // NOTE: Blocks button state read.
  // while (client.available()) {
  //   String line = client.readStringUntil('\r');
  //   Serial.print(line);
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