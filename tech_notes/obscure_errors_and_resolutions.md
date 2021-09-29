### WPS error

By default, M5ez supports "WPS pushbutton" and "WPS pincode" as ways to connect to Wifi. If you run into an error saying ...

```
Documents\Arduino\libraries\M5ez-master\src\M5ez.cpp:1319:19:
error: 'struct esp_wps_config_t' has no member named 'factory_info'
```

... it probably means your ESP32 library is out of date. Espressif (the ESP32 people) changed how WPS is done in late June 2018. If this happens to you and you would like to have WPS, simply follow the instructions [here](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md) to make sure you have the latest ESP32 Arduino support and the problem should go away.

> *Alternatively, if you do not plan to use WPS, you can also comment out `#define M5EZ_WPS` in the `M5ez.h` file in the library's directory.**