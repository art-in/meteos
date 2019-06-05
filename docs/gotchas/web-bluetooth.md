1. Web Bluetooth API is still experimental, and spec is in draft.
1. Only browser supporting Web Bluetooth API is Chrome.
1. Does not work [without https](https://developers.google.com/web/updates/2015/07/interact-with-ble-devices-on-the-web#https_only).
1. Chrome depends heavily on BLE support from OS:
    - on Windows 10 it requires system update v1703+ (otherwise ["NotFoundError: Bluetooth adapter not available"](https://bugs.chromium.org/p/chromium/issues/detail?id=945182)).
    - on Android it requires v6+
1. Windows 10 (or Chrome) caching BT device info by its MAC address.
    So when changing services/characteristics on the device, `.getPrimaryService()` / `.getCharacteristic()` may fail with `DOMException`.  
    Fix: re-add device in Windows BT devices menu.
