1. Web Bluetooth API is still experimental, and spec is in draft.
2. Only browser supporting Web Bluetooth API is Chrome.
3. Chrome only supports BLE (Bluetooth Low Energy) at the moment (no classic BR/EDR).  
    Althrough spec says "The first version of this specification allows web pages, running on a UA in the Central role, to connect to GATT Servers over either a BR/EDR or LE connection."
4. Chrome depends heavily on OS BLE support:
    - on Windows 10 it requires system update v1703+ (otherwise ["NotFoundError: Bluetooth adapter not available"](https://bugs.chromium.org/p/chromium/issues/detail?id=945182)).
    - on Android it requires v6+