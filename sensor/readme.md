Meteos Sensor

Firmware for ESP32 microcontroller that periodically reads environment data from sensors (BME280, MH-Z19) and sends it to Backend over wifi.

[Sensor device (components, circuit diagram, etc.)](../docs/sensor/readme.md)

---

Build
---

1. Install [Arduino IDE](https://www.arduino.cc/en/main/software#download) (+ [vscode-arduino](https://github.com/Microsoft/vscode-arduino) if vscode fan)
1. Add [Arduino core for the ESP32](https://github.com/espressif/arduino-esp32)
1. Set Board Configuration:  
    - Board: "ESP32 Dev Module (esp32)"
    - Partition Scheme: "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"
    - CPU Frequency: "80MHz(WiFi/BT)"
1. Connect ESP32 to computer over USB
1. Compile / Upload

Notes: tested with Arduino IDE v1.8.9, ESP32 core v1.0.2, all libraries added to `src/libs`.
