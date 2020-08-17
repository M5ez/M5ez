/*
 * 
 * This is a "z-sketch". It means you can run this sketch on its own, or use it as a sub-sketch of some bigger program
 * See the M5ez user manual under z-sketches at https://github.com/M5ez/M5ez
 *
 */

#ifndef MAIN_DECLARED

#include <M5Stack.h>
#include <M5ez.h>

void setup() {
  ez.begin();
  sysInfo();
}

void loop() {

}

String exit_button = "";

#else

String exit_button = "Exit";

#endif  // #ifndef MAIN_DECLARED



void sysInfo() {
  sysInfoPage1();
  while(true) {
    String btn = ez.buttons.poll();
    if (btn == "up") sysInfoPage1();
    if (btn == "down") sysInfoPage2();
    if (btn == "Exit") break;
  }
}

#include <SPIFFS.h>

void sysInfoPage1() {
  const byte tab = 120;
  ez.screen.clear();
  ez.header.show("System Info  (1/2)");
  ez.buttons.show("#" + exit_button + "#down");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);
  ez.canvas.println("");
  ez.canvas.print("CPU freq:");  ez.canvas.x(tab); ez.canvas.println(String(ESP.getCpuFreqMHz()) + " MHz");
  ez.canvas.print("CPU cores:");  ez.canvas.x(tab); ez.canvas.println("2");    //   :)
  ez.canvas.print("Chip rev.:");  ez.canvas.x(tab); ez.canvas.println(String(ESP.getChipRevision()));
  ez.canvas.print("Flash speed:");  ez.canvas.x(tab); ez.canvas.println(String(ESP.getFlashChipSpeed() / 1000000) + " MHz");
  ez.canvas.print("Flash size:");  ez.canvas.x(tab); ez.canvas.println(String(ESP.getFlashChipSize() / 1000000) + " MB");
  ez.canvas.print("ESP SDK:");  ez.canvas.x(tab); ez.canvas.println(String(ESP.getSdkVersion()));
  ez.canvas.print("M5ez:");  ez.canvas.x(tab); ez.canvas.println(String(ez.version()));
}

void sysInfoPage2() {
  const String SD_Type[5] = {"NONE", "MMC", "SD", "SDHC", "UNKNOWN"};
  const byte tab = 140;
  ez.screen.clear();
  ez.header.show("System Info  (2/2)");
  ez.buttons.show("up#" + exit_button + "#");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);
  ez.canvas.println("");
  ez.canvas.print("Free RAM:");  ez.canvas.x(tab);  ez.canvas.println(String((long)ESP.getFreeHeap()) + " bytes");
  ez.canvas.print("Min. free seen:");  ez.canvas.x(tab); ez.canvas.println(String((long)esp_get_minimum_free_heap_size()) + " bytes");
  const int sd_type = SD.cardType();
  
  SPIFFS.begin();
  ez.canvas.print("SPIFFS size:"); ez.canvas.x(tab); ez.canvas.println(String((long)SPIFFS.totalBytes()) + " bytes");
  ez.canvas.print("SPIFFS used:"); ez.canvas.x(tab); ez.canvas.println(String((long)SPIFFS.usedBytes()) + " bytes");
  ez.canvas.print("SD type:"); ez.canvas.x(tab); ez.canvas.println(SD_Type[sd_type]);
  if (sd_type != 0) {
    ez.canvas.print("SD size:"); ez.canvas.x(tab); ez.canvas.println(String((long)SD.cardSize()  / 1000000) + " MB");
    ez.canvas.print("SD used:"); ez.canvas.x(tab); ez.canvas.println(String((long)SD.usedBytes()  / 1000000) + " MB");
  }
}
