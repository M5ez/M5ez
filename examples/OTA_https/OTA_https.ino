#include <M5Stack.h>
#include <M5ez.h>

void setup() {
  ez.begin();
  ezMenu OTA_menu ("OTA via https demo");
  OTA_menu.addItem("Settings", ez.settings.menu);
  OTA_menu.addItem("Get M5ez demo", OTA_demo);
  OTA_menu.run();
}

void loop() {
}

void OTA_demo() {  
  if (ez.msgBox("Get M5ez demo program", "This will download and replace the current sketch with the M5ez demo program.", "Cancel#OK#") == "OK") {
    ezProgressBar progress_bar("OTA update in progress", "Downloading ...", "Abort");
    #include "raw_githubusercontent_com.h" // the root certificate is now in const char * root_cert
    if (ez.wifi.update("https://raw.githubusercontent.com/M5ez/M5ez/master/compiled_binaries/M5ez-demo.bin", root_cert, &progress_bar)) {
      ez.msgBox("Over The Air updater", "OTA download successful. Reboot to new firmware", "Reboot");
      ESP.restart();
    } else {
      ez.msgBox("OTA error", ez.wifi.updateError(), "OK");
    }
  }
}
