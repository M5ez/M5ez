/*
 *  To use this sketch, you need to use the Arduino ESP32 filesystem uploader to upload the 
 *  images from the data directory to the SPIFFS partition on your ESP32. See
 *  
 *  https://github.com/me-no-dev/arduino-esp32fs-plugin for the uploader
 * 
 */

#include <M5Stack.h>
#include <M5ez.h>
#include <SPIFFS.h>

#include "images.h"

void setup() {

  m5.begin();
  SPIFFS.begin();

}

void loop() {
  ezMenu images;
  images.imgBackground(TFT_BLACK);
  images.imgFromTop(40);
  images.imgCaptionFont(&FreeSansBold12pt7b);
  images.imgCaptionLocation(TC_DATUM);
  images.imgCaptionColor(TFT_WHITE);
  images.imgCaptionMargins(10,10);
  images.addItem(wifi_jpg, "WiFi Settings", ezWifiMenu);
  images.addItem(SPIFFS, "/rose.jpg", "Rose");
  images.addItem(SPIFFS, "/galaxy.jpg", "Galaxy");
  images.run();
}
