#ifndef _M5EZ_CONFIG_H_
#define _M5EZ_CONFIG_H_

/*

M5ez configuration file. You can change library options here.
You can turn off option by commenting it out with // or changing (1) to (0)

CAUTION! This file WILL be overwritten on library udpate. Please back it
up before updating.

If you are using PlatformIO, copy and adjust following lines into your platformio.ini
file instead of changing this file.

build_flags=
  -DM5EZ_USER_CONFIG
  -DM5EZ_WPS=1
  -DM5EZ_WIFI=1
  -DM5EZ_BATTERY=1
  -DM5EZ_BLE=0
  -DM5EZ_WIFI_DEBUG=0
  -DM5EZ_BACKLIGHT=1
  -DM5EZ_CLOCK=1
  -DM5EZ_FACES=1

*/

// keep this line here for PlatformIO usage
#ifndef M5EZ_USER_CONFIG

// Turn this off to disable WPS.
#define M5EZ_WPS (1)

// Turn this off to compile without WiFi (no) OTA updates, no clock)
#define M5EZ_WIFI (1)

// Turn this off if you don't have a battery attached
#define M5EZ_BATTERY (1)

// Turn this off to compile without BLE (Bluetooth Low Energy)
//#define M5EZ_BLE (1)

#define M5EZ_BLE_DEVICE_NAME "M5ez"

// Have the autoconnect logic print debug messages on the serial port
//#define M5EZ_WIFI_DEBUG (1)

// Determines whether the backlight is settable
#define M5EZ_BACKLIGHT (1)

// Compile in ezTime and create a settings menu for clock display
#define M5EZ_CLOCK (1)

// FACES settings menu
#define M5EZ_FACES (1)


#endif // M5EZ_USER_CONFIG

#endif // _M5EZ_CONFIG_H_
