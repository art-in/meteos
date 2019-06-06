Home weather station on [ESP32](https://en.wikipedia.org/wiki/ESP32).

---

### Features

- environment readings:
  - temperature
  - humidity
  - pressure
  - co2
 - sending readings to database over wifi
 - configuring sensor over bluetooth
 - on-board display to show current readings
 - browser web app to show graphs

---

### How it looks


<img src="docs/sensor/in-hand.jpg" width="400" /> | <img src="https://via.placeholder.com/400x340.png" width="400" />
:---: | :---:
sensor | web app (TODO)

---

### How it works

<a href="docs/architecture/meteos-architecture.pdf">
<img src="docs/architecture/meteos-architecture.png" />
</a>

---

### Sensor

module | model | link | photo 
--- | --- | --- | ---
microcontroller | esp32 | [aliexpress](http://aliexpress.com/item/ESP32-ESP-32-ESP32S-ESP-32S-CP2102-Wireless-WiFi-Bluetooth-Development-Board-Micro-USB-Dual-Core/32867696371.html) | <img src="docs/sensor/parts/esp32-dev-board.jpg" width="200" />
co2 sensor | mh-z19 | [aliexpress](http://aliexpress.com/item/1PCS-module-MH-Z19-infrared-co2-sensor-for-co2-monitor-Free-shipping/32371956420.html) | <img src="docs/sensor/parts/mh-z19.jpg" width="200" />
temperature, humidity, pressure sensor | bme280 | [aliexpress](http://aliexpress.com/store/product/BME280/731260_32849462236.html) | <img src="docs/sensor/parts/bme280.jpg" width="200" />
OLED display (1.3", 128 x 64) | sh1106 | [aliexpress](http://aliexpress.com/item/1PCS-1-3-OLED-module-white-color-128X64-1-3-inch-OLED-LCD-LED-Display-Module/32796086867.html) | <img src="docs/sensor/parts/sh1106.jpg" width="200" />
li-ion battery (18650) | ncr18650b | [aliexpress](http://aliexpress.com/item/2PCS-100-New-Original-NCR18650B-3-7-v-3400-mah-18650-Lithium-Rechargeable-Battery-Flashlight-Digital/32324914059.html) | <img src="docs/sensor/parts/ncr18650b.jpg" width="200" />
li-ion battery charging board with protection | tp4056 | [aliexpress](http://aliexpress.com/item/1PCS-5V-1A-Micro-USB-18650-Lithium-Battery-Charging-Board-Charger-Module-Protection-Dual-Functions/32467578996.html) | <img src="docs/sensor/parts/tp4056.jpg" width="200" />
battery case (18650) | - | [aliexpress](http://aliexpress.com/item/High-Quality-18650-Battery-Box-Holder-Batteries-Case-for-4pcs-18650-in-Parallel-3-7V-Pole/32814062027.html) | <img src="docs/sensor/parts/battery-case.jpg" width="200" />
step-up voltage converter (1V-5V to 5V) | - | [aliexpress](http://aliexpress.com/item/5V-DC-DC-Converter-Step-Up-Power-Supply-DC-DC-Booster-Boost-Buck-Converter-Board-Step/32635991770.html) | <img src="docs/sensor/parts/stepup-converter.jpg" width="200" />
capacitor (1K μF) | - | -
button | - | - | <img src="docs/sensor/parts/button.jpg" width="200" />
plastic case (105 х 75 х 26.4 mm) | - | [chipdip (RU)](https://www.chipdip.ru/product/g909g) | <img src="docs/sensor/parts/case1.jpg" width="200" /> <br/> <img src="docs/sensor/parts/case2.jpg" width="200" />

total cost (including shipment from China to Saint-Petersburg, Russia): 2.700 RUB (42 USD)  
most expensive module: co2 sensor - 1.230 RUB (19 USD)  
shipment time: 2-4 weeks

---

assembling  |  _
-- | --
electric schema (TODO) | <img src="https://via.placeholder.com/400x340.png" width="400" />
assembled on breadboard | <img src="docs/sensor/on-breadboard.jpg" width="400" />
soldered & glued inside case | <img src="docs/sensor/case-internals.jpg" width="400" />
result | <img src="docs/sensor/on-table.jpg" width="400" />
