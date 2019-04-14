1. Web Bluetooth API is still experimental, and spec is in draft.
2. Only browser supporting Web Bluetooth API is Chrome.
3. Chrome supports both classic BR/EDR and BLE (Bluetooth Low Energy).
4. Chrome depends heavily on BLE support from OS:
    - on Windows 10 it requires system update v1703+ (otherwise ["NotFoundError: Bluetooth adapter not available"](https://bugs.chromium.org/p/chromium/issues/detail?id=945182)).
    - on Android it requires v6+