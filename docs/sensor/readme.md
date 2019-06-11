### Components

component | model | link | photo 
--- | --- | --- | ---
microcontroller | esp32 doit devkit v1 | [aliexpress](http://aliexpress.com/item/ESP32-ESP-32-ESP32S-ESP-32S-CP2102-Wireless-WiFi-Bluetooth-Development-Board-Micro-USB-Dual-Core/32867696371.html) | <img src="photos/parts/esp32-dev-board.jpg" width="200" />
co2 sensor | mh-z19b | [aliexpress](http://aliexpress.com/item/1PCS-module-MH-Z19-infrared-co2-sensor-for-co2-monitor-Free-shipping/32371956420.html) | <img src="photos/parts/mh-z19.jpg" width="200" />
temperature, humidity, pressure sensor | bme280 | [aliexpress](http://aliexpress.com/store/product/BME280/731260_32849462236.html) | <img src="photos/parts/bme280.jpg" width="200" />
OLED display (1.3", 128 x 64) | sh1106 | [aliexpress](http://aliexpress.com/item/1PCS-1-3-OLED-module-white-color-128X64-1-3-inch-OLED-LCD-LED-Display-Module/32796086867.html) | <img src="photos/parts/sh1106.jpg" width="200" />
li-ion battery (18650) | ncr18650b | [aliexpress](http://aliexpress.com/item/2PCS-100-New-Original-NCR18650B-3-7-v-3400-mah-18650-Lithium-Rechargeable-Battery-Flashlight-Digital/32324914059.html) | <img src="photos/parts/ncr18650b.jpg" width="200" />
li-ion battery charging board with protection | tp4056 | [aliexpress](http://aliexpress.com/item/1PCS-5V-1A-Micro-USB-18650-Lithium-Battery-Charging-Board-Charger-Module-Protection-Dual-Functions/32467578996.html) | <img src="photos/parts/tp4056.jpg" width="200" />
battery case (18650) | - | [aliexpress](http://aliexpress.com/item/High-Quality-18650-Battery-Box-Holder-Batteries-Case-for-4pcs-18650-in-Parallel-3-7V-Pole/32814062027.html) | <img src="photos/parts/battery-case.jpg" width="200" />
step-up voltage converter (1V-5V to 5V) | - | [aliexpress](http://aliexpress.com/item/5V-DC-DC-Converter-Step-Up-Power-Supply-DC-DC-Booster-Boost-Buck-Converter-Board-Step/32635991770.html) | <img src="photos/parts/stepup-converter.jpg" width="200" />
capacitor (1K μF) | - | -
button | - | - | <img src="photos/parts/button.jpg" width="200" />
plastic case (105 х 75 х 26.4 mm) | - | [chipdip (RU)](https://www.chipdip.ru/product/g909g) | <img src="photos/parts/case1.jpg" width="200" /> <br/> <img src="photos/parts/case2.jpg" width="200" />

total cost (including shipment from China to Saint-Petersburg, Russia): 2.700 RUB (42 USD)  

most expensive component: co2 sensor - 1.230 RUB (19 USD)  
shipment time: 2-4 weeks

---

### Assembling

<span style="display: none"><span>  |  <span style="display: none"><span>
-- | --
circuit diagram | <a href="circuit/meteos-sensor-circuit.pdf"><img src="circuit/meteos-sensor-circuit.png" width="400" /></a>
assembling on breadboard | <img src="photos/on-breadboard.jpg" width="400" />
soldering / glueing / drilling / packing in case | <img src="photos/case-internals.jpg" width="400" />
connecting to usb for charging / debugging | <img src="photos/connecting-usb.jpg" width="400" />
front side | <img src="photos/on-table.jpg" width="400" /> <br> <img src="photos/on-table-reverse.jpg" width="400" />

---

### Technical Parameters

<span style="display: none"><span>  |  <span style="display: none"><span>
-- | --
operating temperature range | 0 ~ 50 °C
weight | 136 g
average current | ~ 90 mA
on-battery life time | ~ 37 hours

---

### Design Failures

- short on-battery life time (despite all my attempts [#34](https://github.com/art-in/meteos/issues/34)).  
    originally i've expected device to work at least 1 week on single battery charge. but it all comes down to this:
    - esp32 drains considerable amount of current for wifi operations (even in modem sleep mode).
    - mh-z19 was not designed for low-power consumption as it drains min 5 mA constantly with 100 mA peaks.
    
- temperature reading is affected by in-case heating.  
    esp32 with wifi-on radiates a lot of heat, and a bit more comes from mh-z19.  
    despite i've drilled a lot of holes on front panel and a side, looks like it's still not enough.  
    overall it adds up to ~1.8 °C gradually heating up for 4 hours after startup, which needs firmware compensation.

